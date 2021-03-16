#pragma once

#include <Arduino.h>
#include "defines.h"

enum PTT_STATE {PTT_INACTIVE, PTT_DOWN, PTT_UP};

extern volatile bool tick_1Hz;
extern volatile bool tick_125Hz;
extern volatile bool tick_touchscreen;
extern volatile byte tx_timeout;
extern volatile bool cw_tx_change_state;

extern volatile PTT_STATE ptt_state;

void heartbeat_begin(void);
void iambic_key(void);
void hand_key(void);
void winkeyer_key(void);
#ifdef USE_TUNE
void tune(void);
#endif