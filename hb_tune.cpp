#include "heartbeat.h"
#include "radio.h"

#ifdef USE_TUNE
void tune(void) {
  static uint16_t tune_counter = 0;
  if (tune_counter < 10) {
    if (tune_counter == radio_obj.tune_pwr)
      digitalWrite(CW_KEY, false);
    tune_counter++;
  } else {
    digitalWrite(CW_KEY, true);
    tune_counter = 0;  
  }
}
#endif