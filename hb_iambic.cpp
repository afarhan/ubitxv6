#include "heartbeat.h"
#include "radio.h"

const byte NONE_BIT = 0;
const byte DOT_BIT  = _BV(0);
const byte DASH_BIT = _BV(1);
const byte BOTH_BIT = DOT_BIT | DASH_BIT;

const byte MILLIS_SHIFT = 3;   // 8 msec

void iambic_key(void) {
  static int keyer_counter;
  static bool in_tone;
  static bool have_alternate_paddle;
  static byte current;

  register int paddle_read = analogRead(ANALOG_KEYER);

  byte paddle = paddle_read < PS_RIGHT_MAX
    ? paddle_read < PS_BOTH_MAX ? BOTH_BIT : DOT_BIT 
    : paddle_read < PS_LEFT_MAX ? DASH_BIT : NONE_BIT;

  if (!keyer_counter) {
    byte next;
    if (have_alternate_paddle) {
      have_alternate_paddle = false;
      next = current ^ BOTH_BIT;
    } else {
      next = paddle == BOTH_BIT ? (current ^= BOTH_BIT) : paddle;
    }
    current = next;
    if (next) {
      keyer_counter = radio_obj.cw_millis >> MILLIS_SHIFT;
      if (next & DASH_BIT) keyer_counter *= 3;
      keyer_counter++;
      in_tone = true;
      if (!radio_obj.cw_tx) {
        radio_obj.set_cw_tx(true);
        cw_tx_change_state = true;
      }  
      radio_obj.sidetone(true);
      digitalWrite(CW_KEY, radio_obj.cw_tx);
    }
  }

  if (keyer_counter) {
    if ((current | paddle) == BOTH_BIT)
      have_alternate_paddle = radio_obj.cw_key_type == KT_IB;
    if (!--keyer_counter) {
      if (in_tone) {
        // tone done
        in_tone = false;
        keyer_counter = (radio_obj.cw_millis >> MILLIS_SHIFT) - 1;
        radio_obj.sidetone(false);
        digitalWrite(CW_KEY, false);
      } else {
        tx_timeout = ((radio_obj.cw_delaytime * CW_DELAY_MULTIPLE) >> 3) + 1;
      }
    }
  }
}
