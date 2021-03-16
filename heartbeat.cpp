#include "heartbeat.h"
#include "rotary.h"
#include "radio.h"

#include "defines.h"
#include "pins.h" 

volatile bool tick_1Hz;
volatile bool tick_125Hz;
volatile bool tick_touchscreen;

volatile PTT_STATE ptt_state;
volatile byte tx_timeout;
volatile bool cw_tx_change_state;

const byte PS2_1024 = _BV(CS22) | _BV(CS21) | _BV(CS20);
const byte PS2_256  = _BV(CS22) | _BV(CS21);



void heartbeat_begin(void) {
  // set timer2 interrupt at 125 Hz
  TCNT2  = 0;                                     // initialize counter
  TCCR2A = _BV(WGM21);                            // turn on CTC mode
  TCCR2B = PS2_1024;                              // set 1024 prescaler
  OCR2A  = (byte)(F_CPU / (1024L * 125L) - 1L);   // set top (must be < 256)
  TIMSK2 = _BV(OCIE2A);                           // enable timer compare interrupt

  // setup (But don't activate) timer0 for sidetone on OC0A output (CW_TONE)
  pinMode(CW_TONE, OUTPUT);
  digitalWrite(CW_TONE, 0);
  TCNT0  = 0;                                      // initialize counter 
  TCCR0A = _BV(WGM01);                             // turn on CTC mode
  TCCR0B = _BV(CS02);                              // set 256 prescaler

  // setup analog inpurt for keyer
  analogReference(DEFAULT);
  pinMode(ANALOG_KEYER, INPUT_PULLUP);
}


ISR(TIMER2_COMPA_vect) {    
  const byte T2_CT = 124;            // cycles to skip for 1Hz
  static byte t2_ct = T2_CT;

  const byte TOUCHSCREEN_CT = 9;     // cycles to skip for touchscreen (80 ms)
  static byte touchscreen_ct = TOUCHSCREEN_CT;

  /*
   * The millis() function depends on a timer0 interrupt updating
   * timer0_millis. Since we replace this timer0 interrupt (for sidetone)
   * we manuall update timer0_millis here. This way functions (mainly 
   * library functions) that depend on millis() will still work.
   */                                  
  extern volatile unsigned long timer0_millis; 
  timer0_millis += 8;   // 125 Hz = 8 msec
             
  tick_125Hz = true;
  if (!touchscreen_ct--) {
    touchscreen_ct = TOUCHSCREEN_CT;
    tick_touchscreen = true;
  }

  if (radio_obj.vfo_data[radio_obj.using_vfo_b].cw) {
    switch (radio_obj.cw_key_type) {
      case KT_HAND:
        hand_key();
        break;
      case KT_IA:
      case KT_IB:
        iambic_key();
        break;
      #ifdef USE_WINKEY
        case KT_WK:
          winkeyer_key();
          break;
      #endif
    }
   
    if (tx_timeout) {
      if (!--tx_timeout) {
        radio_obj.set_cw_tx(false);
        cw_tx_change_state = true;
      }
    }
  } else {
    if (ptt_change_millis && (timer0_millis - ptt_change_millis) > SWITCH_DEBOUNCE) {
      ptt_change_millis = 0;
      ptt_state = last_ptt ? PTT_UP : PTT_DOWN; 
    } 
  } 

  #ifdef ROTARY_X2
  if (timer0_millis - encoder_change_millis > ENCODER_DEBOUNCE) {
    byte change = old_encoder ^ new_encoder;
    if ((change > 0) && (change < ROTARY_BITS)) {
      encoder_change_millis = 0;
      old_encoder = new_encoder;
      bool comp = (bool)(new_encoder & ROTARY_BIT_A) != (bool)(new_encoder & ROTARY_BIT_B);
      rotary_state = (change & ROTARY_BIT_A) ? (comp ? R_INC : R_DEC)
                                             : (comp ? R_DEC : R_INC);
    }
  }
  #endif

  #ifdef USE_TUNE
    if (radio_obj.tune)
      tune();
  #endif

  if (!t2_ct--) {
    // 1 Hz - do once every 125 interrupts
    t2_ct = T2_CT;
    tick_1Hz = true;
    #ifdef USE_AUTO_SAVE
    radio_obj.check_autosave();
    #endif
  }
}