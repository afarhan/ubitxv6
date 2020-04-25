#pragma once

#include "settings.h"

/* these are functions implemented in the main file named as ubitx_xxx.ino */
void saveVFOs();
void setFrequency(const unsigned long freq, const bool transmit = false);
void startTx(TuningMode_e tx_mode);
void stopTx();
void ritEnable(unsigned long f);
void ritDisable();
void checkCAT();
void cwKeyer(void);
void switchVFO(Vfo_e vfoSelect);

/* these are functiosn implemented in ubitx_si5351.cpp */
void si5351bx_setfreq(uint8_t clknum, uint32_t fout);
void initOscillators();
void si5351_set_calibration(int32_t cal); //calibration is a small value that is nudged to make up for the inaccuracies of the reference 25 MHz crystal frequency 

bool autoSelectSidebandChanged(const uint32_t old_frequency); //if the current frequency defaults to a different sideband mode, updates to that sideband mode and returns true. Else, returns false
