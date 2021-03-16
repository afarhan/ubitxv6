#pragma once

#include "defines.h"

#include <Arduino.h>

const byte BUTTON_KEYER      = 13;
const byte BUTTON_TOUCH      = 15;
const byte BUTTON_FREQ       = 18;
const byte BUTTON_BFO        = 19;
const byte BUTTON_DELAY      = 17;
#ifdef USE_TUNE
const byte BUTTON_TUNE_PWR   = 16;
#endif
#ifdef USE_PDL_POL
const byte BUTTON_PADDLE     = 14;
#endif
#ifdef USE_TX_DIS
const byte BUTTON_TX_DISABLE = 20;
#endif
const byte BUTTON_CWP        = 12;
const byte BUTTON_CANCEL     = 10;
const byte BUTTON_SAVE       = 11;
