#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "fluidlite.h"
#include "fluid_synth.h"
#include "misc.h"
#include <float.h> // 用于定义 FLT_MIN

#define RED_TEXT(x) "\033[31m" #x "\033[0m"


#define MIRCO_SECOND 1000000
#define SAMPLE_RATE 44100
#define NUM_FRAMES 44100                    // SAMPLE_RATE*DURATION
#define DURATION (NUM_FRAMES / SAMPLE_RATE) // second
#define NUM_CHANNELS 2
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)


// 计算并打印平均音量和最大音量
void calculateAndPrintVolume(int16_t *pcmData, int length) {
    if (length == 0) {
        printf("Error: PCM data length is zero.\n");
        return;
    }

    float maxVolumeDB = -FLT_MAX; // 最大音量
    float sumVolumeDB = 0.0;      // 音量总和

    // 分段计算音量（例如每 1024 个采样点计算一次）
    int segmentSize = 1024;
    for (int i = 0; i < length; i += segmentSize) {
        int segmentLength = (i + segmentSize < length) ? segmentSize : length - i;
        float volumeDB = calculateVolumeDB(pcmData + i, segmentLength);

        // 更新最大音量
        if (volumeDB > maxVolumeDB) {
            maxVolumeDB = volumeDB;
        }

        // 累加音量
        sumVolumeDB += volumeDB;
    }

    // 计算平均音量
    float meanVolumeDB = sumVolumeDB / (length / segmentSize);

    // 打印结果
    printf("mean_volume: %.1f dB\n", meanVolumeDB);
    printf("max_volume: %.1f dB\n", maxVolumeDB);
}


int main(int argc, char *argv[])
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_settings_setstr(settings, "synth.verbose", "no"); // 在新版本中"synth.verbose"是int型
    fluid_settings_setint(settings, "synth.polyphony", 3);
    fluid_settings_setint(settings, "synth.midi-channels", 1);
    fluid_settings_setstr(settings, "synth.reverb.active", "no");

    fluid_synth_t *synth = new_fluid_synth(settings);
    int sfont = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(sizeof(int16_t) , NUM_SAMPLES);

    float pre_db = -FLT_MAX;

	for(int i=10; i<=120; i+=10){
        fluid_synth_noteon(synth, 0, C, i);
        fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, 1, buffer, 1, 2);

        float cur_db = calculateVolumeDB(buffer, NUM_SAMPLES);
        printf(RED_TEXT(Velocity) " %d:\tMEAN DB %.1f  ,\tPEAK DB:%.1f\n", i, cur_db, calculate_peak_dB(buffer, NUM_SAMPLES));
        assert(cur_db > pre_db);
        pre_db = cur_db;
        calculateAndPrintVolume(buffer, NUM_SAMPLES);

        if(false){
            char *fname = "velocity.pcm";
            FILE* file = fopen(fname, "wb");
            fwrite(buffer, sizeof(uint16_t), NUM_SAMPLES, file);
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 2 -i velocity.pcm velocity%d.wav", i);
            system(cmd);
        }
    }


    free(buffer);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return 0;
}

