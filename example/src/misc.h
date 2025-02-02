#ifndef _MISC_H
#define _MISC_H

#include <stdint.h>
#include <stdbool.h>

#define TEST_SUCCESS(FLUID_FUNCT) TEST_ASSERT((FLUID_FUNCT) != FLUID_FAILED)

bool float_eq(double a, double b);

void set_bit(uint8_t* value, uint8_t index);

void unset_bit(uint8_t* value, uint8_t index);

void change_bit(uint8_t* value, uint8_t index, uint8_t bit);



// 计算音量（dB）
float calculateVolumeDB(int16_t *pcmData, int length);

float calculate_peak_dB(int16_t *pcmData, int length);

float calculate_peak_dB_1024(int16_t *pcmData, int length);


#endif /* _MISC_H */

