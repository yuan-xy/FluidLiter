#include <stdio.h>
#include "misc.h"

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

#include <math.h>
#include <float.h> // 用于定义 FLT_MIN

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

