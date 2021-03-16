#pragma once

#include <Arduino.h>
#include "pins.h"

const byte SWITCH_DEBOUNCE = 100;   // minimum milliseconds short press / PTT change
const byte ENCODER_DEBOUNCE = 15;

const unsigned LONG_DOWN = 1000;    // minimum milliseconds long press of rotary switch

const byte ROTARY_BIT_A = _BV(digitalPinToPCMSKbit(SW1_A));
const byte ROTARY_BIT_B = _BV(digitalPinToPCMSKbit(SW1_B));
const byte ROTARY_BITS = ROTARY_BIT_A | ROTARY_BIT_B;

extern volatile unsigned long sw_down_millis;
extern volatile bool need_sw_beep;


enum ROTARY_STATE {R_INACTIVE, R_INC, R_DEC};
enum SWITCH_STATE {S_INACTIVE, S_SHORT, S_LONG};

void rotary_begin(void);
extern volatile ROTARY_STATE rotary_state;
extern volatile SWITCH_STATE switch_state;
extern volatile unsigned long ptt_change_millis;
extern volatile byte last_ptt;

#ifdef ROTARY_X2
extern volatile unsigned long encoder_change_millis;
extern volatile byte old_encoder;
extern volatile byte new_encoder;
#endif