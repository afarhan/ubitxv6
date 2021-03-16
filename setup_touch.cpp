#include "setup.h"
#include "setup_buttons.h"
#include "radio.h"

bool setup_loop::touch(const byte button) {
  switch (button) {
    case BUTTON_CWP:
      return !(
               radio_obj.in_freq_cal
           ||  radio_obj.in_bfo_cal
           ||  radio_obj.in_cw_delay
           #ifdef USE_TUNE
           ||  radio_obj.in_tune_pwr
           #endif
           );
      break;
    #ifdef USE_TUNE
      case BUTTON_TUNE_PWR:
        return !(
                radio_obj.in_cw_pitch
            ||  radio_obj.in_freq_cal
            ||  radio_obj.in_bfo_cal
            ||  radio_obj.in_cw_delay
            );
        break;
    #endif
    case BUTTON_DELAY:
      return !(
               radio_obj.in_cw_pitch
           ||  radio_obj.in_freq_cal
           ||  radio_obj.in_bfo_cal
           #ifdef USE_TUNE
           ||  radio_obj.in_tune_pwr
           #endif
           );
      break;
    case BUTTON_FREQ:
      return !(
               radio_obj.in_cw_pitch
           ||  radio_obj.in_bfo_cal
           ||  radio_obj.in_cw_delay
           #ifdef USE_TUNE
           ||  radio_obj.in_tune_pwr
           #endif
           );
      break;
    case BUTTON_BFO:
      return !(
               radio_obj.in_cw_pitch
           ||  radio_obj.in_freq_cal
           ||  radio_obj.in_cw_delay
           #ifdef USE_TUNE
           ||  radio_obj.in_tune_pwr
           #endif
           );
      break;
  }
  return true;
}
