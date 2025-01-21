#ifndef _MISC_H
#define _MISC_H

#include <stdint.h>


void set_bit(uint8_t* value, uint8_t index);

void unset_bit(uint8_t* value, uint8_t index);

void change_bit(uint8_t* value, uint8_t index, uint8_t bit);



// 计算音量（dB）
float calculateVolumeDB(int16_t *pcmData, int length);

double calculate_peak_dB(int16_t *samples, int num_samples);


#endif /* _MISC_H */

