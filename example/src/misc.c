#include <stdio.h>
#include "misc.h"
#include <math.h>
#include <float.h> // 用于定义 FLT_MIN

void set_bit(uint8_t* value, uint8_t index){
	uint8_t bitmask = 1 << index;
	*value |= bitmask;
}

void unset_bit(uint8_t* value, uint8_t index){
	uint8_t bitmask = ~(1 << index);
	*value &= bitmask;
}

void change_bit(uint8_t* value, uint8_t index, uint8_t bit){
	if(bit) set_bit(value, index);
	else unset_bit(value, index);
}



// 计算音量（dB）
float calculateVolumeDB(int16_t *pcmData, int length) {
    if (length == 0) {
        return -FLT_MAX; // 返回最小浮点数表示无效值
    }

    double sum = 0.0;
    for (int i = 0; i < length; i++) {
        sum += (double)pcmData[i] * pcmData[i]; // 计算平方和
    }

    double rms = sqrt(sum / length); // 计算 RMS
    
    // 参考电平的 RMS 值（16 位 PCM 数据的最大值为 32767）
    double rmsRef = 32767.0 / sqrt(2.0);

    // 计算 dB 值（相对于参考电平）
    return 20 * log10(rms / rmsRef);
}

float calculate_peak_dB(int16_t *samples, int num_samples) {
    double peak = 0.0;
    for (int i = 0; i < num_samples; i++) {
        double normalized_sample = samples[i] / 32768.0;
        double abs_sample = fabs(normalized_sample);
        if (abs_sample > peak) {
            peak = abs_sample;
        }
    }
    if (peak == 0.0) {
        return -FLT_MAX;
    }
    return 20 * log10(peak);
}


float calculate_peak_dB_1024(int16_t *pcmData, int length) {
    if (length == 0) {
        return -FLT_MAX;
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
    // printf("mean_volume: %.1f dB\n", meanVolumeDB);
    // printf("max_volume: %.1f dB\n", maxVolumeDB);
    return maxVolumeDB;
}