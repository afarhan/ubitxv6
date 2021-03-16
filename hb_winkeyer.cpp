#include "heartbeat.h"
#include "rotary.h"
#include "radio.h"

// with the winkeyer, PTT is controlled by the RING, CW keying is controlled by the TIP


void winkeyer_key(void) {
  static bool old_ptt = false;
  static bool old_cw = false;
  int value = analogRead(ANALOG_KEYER);

  bool new_state = value < PS_LEFT_MAX;
  if (old_ptt != new_state) {
    old_ptt = new_state;
    radio_obj.set_cw_tx(new_state);
    cw_tx_change_state = true;
  } 
  new_state = value < PS_BOTH_MAX;
  if (old_cw != new_state) {
    old_cw = new_state;
    radio_obj.sidetone(old_cw);
    digitalWrite(CW_KEY, old_cw);
  }
}