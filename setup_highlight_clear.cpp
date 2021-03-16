#include "setup.h"
#include "setup_buttons.h"
#include "radio.h"

void setup_loop::highlight_clear_button(const byte button, bool highlight) {
  if (
        (button == BUTTON_FREQ       && radio_obj.in_freq_cal)
     || (button == BUTTON_BFO        && radio_obj.in_bfo_cal) 
     || (button == BUTTON_CWP        && radio_obj.in_cw_pitch)
     #ifdef USE_TUNE
     || (button == BUTTON_TUNE_PWR   && radio_obj.in_tune_pwr)
     #endif
     #ifdef USE_TX_DIS
     || (button == BUTTON_TX_DISABLE && !radio_obj.tx_enable)
     #endif
     #ifdef USE_PDL_POL
     || (button == BUTTON_PADDLE     && radio_obj.paddle_reverse)
     #endif
     || (button == BUTTON_DELAY      && radio_obj.in_cw_delay)
    )
    highlight = !highlight;
  loop_master::highlight_clear_button(button, highlight);
};
