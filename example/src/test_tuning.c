#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "fluidlite.h"
#include "fluid_synth.h"
#include "utils.c"
#include "fluid_sfont.h"
#include "fluid_tuning.h"
#include "misc.h"

#define SAMPLE_RATE 44100

void print_voice(fluid_synth_t *synth){
    for(int j=0; j< synth->polyphony; j++){
        if(_PLAYING(synth->voice[j])) printf("pitch%d -> %f\n", j, synth->voice[j]->pitch);
    }
}

int main(int argc, char *argv[]) {
    char *fname = "pcmTuning.pcm";

    FILE *file = fopen(fname, "wb");

    fluid_synth_t *synth = NEW_FLUID_SYNTH();

    int sfid = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfid, 0, 0);

    double ratio[] = {1, 16. / 15, 9. / 8, 6. / 5, 5. / 4, 4. / 3, 45. / 32, 3. / 2, 8. / 5, 5. / 3, 9. / 5, 15. / 8};
    assert(sizeof(ratio) / sizeof(ratio[0] == 12));

    double pitch[12];
    for (int i = 0; i < 12; i++) {
        double hz = 261.625565 * ratio[i];
        double ct = fluid_hz2ct(hz);
        pitch[i] = ct - (6000 + 100 * i);
        printf("%d:%fhz, %fct, %fpitch, %fratio\n", i, hz, ct, pitch[i], ratio[i]);
    }

    for (int i = 0; i < 12; i++) {
        double ct = 6000 + 100 * i + pitch[i];
        double hz = fluid_ct2hz(ct);
        double r = hz / 261.625565;
        assert(float_eq(r, ratio[i]));
    }

    int notes[] = {NN1, NN3, NN5};
    int frame = SAMPLE_RATE * 2;
    int16_t buffer[frame];

    fluid_synth_noteon(synth, 0, NN6, 127);
    fluid_synth_write_s16_mono(synth, frame, buffer);
    fwrite(buffer, sizeof(int16_t), frame, file);
    print_voice(synth);

    for (int i = 0; i < 3; i++) {
        int note = notes[i];
        fluid_synth_noteon(synth, 0, note, 127);
    }
    fluid_synth_write_s16_mono(synth, frame, buffer);
    fwrite(buffer, sizeof(int16_t), frame, file);
    print_voice(synth);
    for (int i = 0; i < 3; i++) {
        int note = notes[i];
        fluid_synth_noteoff(synth, 0, note);
    }


    int notes8[] = {NN1, NN2, NN3, NN4, NN5, NN6, NN7, NN1+12};

    for (int i = 0; i < 8; i++) {
        int note = notes8[i];
        fluid_synth_noteon(synth, 0, note, 127);
        fluid_synth_write_s16_mono(synth, frame/4, buffer);
        fwrite(buffer, sizeof(int16_t), frame/4, file);
        fluid_synth_noteoff(synth, 0, note);
        print_voice(synth);
    }


    fclose(file);
    system("ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 1 -i pcmTuning.pcm pcmTuningNo.wav");





    file = fopen(fname, "wb");

    assert(synth->tuning == NULL);
    int ret = fluid_synth_create_octave_tuning(synth, 0, 0, "Just Intonation", pitch);
    assert(ret == FLUID_OK);
    assert(synth->tuning != NULL);
    assert(synth->tuning[0][0] != NULL);
    fluid_tuning_t *tn = synth->tuning[0][0];
    printf("name:%s, bank:%d, prog:%d\n", tn->name, tn->bank, tn->prog);
    for(int i=0; i<128; i++){
        // printf("%i:%f\n", i*100, tn->pitch[i]);
    }
    printf("\n");
    assert(synth->voice[0]->channel->tuning == NULL);

    synth->channel[0]->tuning = synth->tuning[0][0];

    fluid_tuning_t *tuning = fluid_channel_get_tuning(synth->voice[0]->channel);
    assert(fluid_channel_has_tuning(synth->voice[0]->channel));
    assert(fluid_tuning_get_pitch(tuning, 60) == 6000);
    assert(fluid_tuning_get_pitch(tuning, 64) == 6386.3134765625);
    assert(synth->voice[0]->gen[GEN_SCALETUNE].val == 100.0f);
    assert(synth->voice[0]->channel->tuning != NULL);

    fluid_synth_noteon(synth, 0, NN6, 127);
    fluid_synth_write_s16_mono(synth, frame, buffer);
    fwrite(buffer, sizeof(int16_t), frame, file);
    print_voice(synth);

    // 声音没听出差别呢, 但是看频率确实变了
    for (int i = 0; i < 3; i++) {
        int note = notes[i];
        fluid_synth_noteon(synth, 0, note, 127);
    }
    fluid_synth_write_s16_mono(synth, frame, buffer);
    fwrite(buffer, sizeof(int16_t), frame, file);
    print_voice(synth);
    for (int i = 0; i < 3; i++) {
        int note = notes[i];
        fluid_synth_noteoff(synth, 0, note);
    }


    for (int i = 0; i < 8; i++) {
        int note = notes8[i];
        fluid_synth_noteon(synth, 0, note, 127);
        fluid_synth_write_s16_mono(synth, frame/4, buffer);
        fwrite(buffer, sizeof(int16_t), frame/4, file);
        fluid_synth_noteoff(synth, 0, note);
        print_voice(synth);
    }


    fclose(file);
    delete_fluid_synth(synth);
    system("ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 1 -i pcmTuning.pcm pcmTuning2.wav");
    return 0;
}
