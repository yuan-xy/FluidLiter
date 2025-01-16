#ifndef _MISC_H
#define _MISC_H

#include <stdint.h>

void set_bit(uint8_t* value, uint8_t index);

void unset_bit(uint8_t* value, uint8_t index);

void change_bit(uint8_t* value, uint8_t index, uint8_t bit);

#endif /* _MISC_H */
