#pragma once

#include "defines.h"
#include <Arduino.h>

// the only functions from ubitx_si5351.cpp called externally

void si5351bx_init(void);
void si5351bx_setfreq(uint8_t clknum, uint32_t fout);
void si5351_set_calibration(int32_t cal);
void i2cWrite(uint8_t reg, uint8_t val);

#ifdef USE_ATTN
extern uint8_t si5351bx_drive[];
#endif

// void initOscillators();
