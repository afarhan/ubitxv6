#pragma once

#include <Arduino.h>
#include "defines.h"

const int8_t MIN_CWS    = 1;
const int8_t MAX_CWS    = 40;

const int16_t MIN_CWP = 300;
const int16_t MAX_CWP = 1000;

const int MIN_DELAY = 10;
const int MAX_DELAY = 100;

const long MASTER_CAL_MIN = -28000000;
const long MASTER_CAL_MAX = 28000000;

const long BFO_MIN = 11048000;
const long BFO_MAX = 11060000;

const long TUNE_PWR_MIN = 1;
const long TUNE_PWR_MAX = 10;

const long MIN_FREQ = 500000l;
const long MAX_FREQ = 30000000l;
const long DEF_VFOA = 7150000l;
const long DEF_VFOB = 14150000l;

const long MAX_USB_CARRIER = 11060000l;
const long MIN_USB_CARRIER = 11048000l;
const long DEF_USB_CARRIER = 11052000l;

const int16_t MAX_CW_MILLIS = 1200 / MIN_CWS;
const int16_t MIN_CW_MILLIS = 1200 / MAX_CWS;
const int16_t DEF_CW_MILLIS = 1200 / 15;

const int16_t DEF_CWP = 600;
const int DEF_DELAY = 50;

const int16_t RIT_LIMIT = 30000;




#ifdef USE_AUTO_SAVE
const long AUTOSAVE_HOLD = 30 * 1000L;   // msec
#endif
