#pragma once

#include <stdint.h>

static const uint32_t SI5351_MIN_FREQUENCY_HZ =    500000;
static const uint32_t SI5351_MAX_FREQUENCY_HZ = 109000000;

void initOscillators();
void si5351bx_setfreq(uint8_t clknum, uint32_t fout);
void si5351_set_calibration(int32_t cal); //calibration is a small value that is nudged to make up for the inaccuracies of the reference 25 MHz crystal frequency 
