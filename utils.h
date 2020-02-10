#pragma once

#define LIMIT(val,min,max) ((val) < (min)) ? (min) : (((max) < (val)) ? (max) : (val))

void formatFreq(uint32_t freq, char* buff_out, uint16_t buff_size, uint8_t fixed_width = 0);
