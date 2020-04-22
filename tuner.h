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

// limits the tuning and working range of the ubitx between 3 MHz and 30 MHz
static const uint32_t THRESHOLD_USB_LSB = 10000000L;
