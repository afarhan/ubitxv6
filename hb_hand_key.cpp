#include "heartbeat.h"
#include "rotary.h"
#include "radio.h"

void hand_key(void) {
  static bool old_state = false;

  bool down = (analogRead(ANALOG_KEYER) < PS_LEFT_MAX) // paddle in hand key
            || (last_ptt == 0);                        // PTT pressed
  if (old_state != down) {
    old_state = down;
    if (!radio_obj.cw_tx) {
      radio_obj.set_cw_tx(true);
      cw_tx_change_state = true;
    }
    radio_obj.sidetone(down);
    digitalWrite(CW_KEY, down);
    if (!down) {
      tx_timeout = ((radio_obj.cw_delaytime * CW_DELAY_MULTIPLE) >> 3) + 1;
    }
  }
}