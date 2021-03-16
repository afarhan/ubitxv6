#include <Arduino.h>

#include "heartbeat.h"
#include "rotary.h"
#include "loop_master.h"
#include "defines.h"

/*
 * this code handles pin changes for the rotary encoder, the rotary encoder switch and the PTT switch
 * all are on the same bank (PINC)
 * for the rotary encoder switch, short and long presses are detected when the switch is released
 * for the PTT the state and last time are recorded, higher level code will debounce changes
 * for the rotary pulses (CW or CCW) are detected, higher level code processes individual pulses
 * it is assumed that all pulses are processed, no accumulation, no momentum
 */

// interrupt communication
volatile ROTARY_STATE rotary_state = R_INACTIVE;
volatile SWITCH_STATE switch_state = S_INACTIVE;
volatile unsigned long ptt_change_millis = 0;
volatile byte last_ptt = _BV(digitalPinToPCMSKbit(PTT));


#ifdef ROTARY_X2
volatile unsigned long encoder_change_millis = 0;
volatile byte old_encoder;
volatile byte new_encoder;
#endif


volatile unsigned long sw_down_millis = 0;
volatile bool need_sw_beep;



void rotary_begin(void) {
  // set interrupt on pin chanage (rotary)
  pinMode(SW1_A, INPUT_PULLUP);
  pinMode(SW1_B, INPUT_PULLUP);
  pinMode(SW1_S, INPUT_PULLUP);
  pinMode(PTT,   INPUT_PULLUP);

  PCICR = _BV(PCIE1);
  PCMSK1 = ROTARY_BIT_A
         #ifdef ROTARY_X2
         | ROTARY_BIT_B
         #endif
         | _BV(digitalPinToPCMSKbit(SW1_S)) 
         | _BV(digitalPinToPCMSKbit(PTT));
}


void rotary_common(const byte interrupt_register, const unsigned long cur_millis) {
  #ifdef ROTARY_X2
  new_encoder = interrupt_register & ROTARY_BITS;
  if (new_encoder != old_encoder)
    encoder_change_millis = cur_millis;

  #else
  /* rotary encoder
   * --------------
   * For CW rotation, when pin A goes low, pin B is low and when pin A goes high pin B is high.
   * For CCW rotation, when pin A goes low, pin B is high and when pin A goes high pin b is low.
   * Else ignore (contact bounce).
   * 
   * Note: this code assumes that pin A and pin B states can be read using the same register.
   * 
   * Note: this method gives half the resolution (~24 changes per revolution)
   */
  static byte b_a_down = _BV(digitalPinToPCMSKbit(SW1_B));     // pin B state when A goes from high->low
  static byte old_pina = _BV(digitalPinToPCMSKbit(SW1_A));     // previous pin A

  byte pina = interrupt_register & _BV(digitalPinToPCMSKbit(SW1_A));    // current pin A
  if (pina ^ old_pina) {                               // pin A changed?
      old_pina = pina;
      byte pin_b = interrupt_register & _BV(digitalPinToPCMSKbit(SW1_B));  // current pin B
      if (pina) {                                         // pin A low->high?
          if (pin_b ^ b_a_down) {
            rotary_state = pin_b ? R_DEC : R_INC;
           }
      } else {                                            // pin A high->low
          b_a_down = pin_b;
      }
  }
  #endif
}

void switch_common(const byte interrupt_register, const unsigned long cur_millis) {
  // if switch has gone high -> low, store millis()
  // else if switch has gone low -> high, check delta 
  // >= LONG_DOWN - long press
  // else >= SHORT_DOWN - short press
  // else ignore (bounce) 
  static byte old_switch = _BV(digitalPinToPCMSKbit(SW1_S));   // previous switch

  byte pins = interrupt_register & _BV(digitalPinToPCMSKbit(SW1_S));    // current switch
  if (pins ^ old_switch) {
    old_switch = pins;
    if (pins) // high - switch up
    {
      unsigned long delta = cur_millis - sw_down_millis;
      if (delta >= SWITCH_DEBOUNCE) {
        switch_state = (delta >= LONG_DOWN) ? S_LONG : S_SHORT;
        sw_down_millis = 0;
        need_sw_beep = false;
      }
    } else {
      sw_down_millis = cur_millis;
      need_sw_beep = true;
    }
  }                             
}

void ptt_common(const byte interrupt_register, const unsigned long cur_millis) {
  byte pins = interrupt_register & _BV(digitalPinToPCMSKbit(PTT));    // current ptt
  if (pins ^ last_ptt) {
    ptt_change_millis = cur_millis;
    last_ptt = pins;
  }
}


ISR(PCINT1_vect) {
  // pin change interrupt handler
  byte interrupt_register = PINC;
  unsigned long cur_millis = millis();

  rotary_common(interrupt_register, cur_millis);
  switch_common(interrupt_register, cur_millis);
  ptt_common(interrupt_register, cur_millis);
}
