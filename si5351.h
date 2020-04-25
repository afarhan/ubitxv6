#pragma once

void initOscillators();
void si5351bx_setfreq(uint8_t clknum, uint32_t fout);
void si5351_set_calibration(int32_t cal); //calibration is a small value that is nudged to make up for the inaccuracies of the reference 25 MHz crystal frequency 
