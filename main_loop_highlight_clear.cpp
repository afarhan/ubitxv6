#include "main_loop.h"
#include "main_buttons.h"
#include "radio.h"
#include "defines.h"

void main_loop::highlight_clear_button(const byte button, bool highlight) {
  if  ((button == BUTTON_RIT   && radio_obj.rit)
    || (button == BUTTON_SPLIT && radio_obj.split)
    #ifdef USE_TUNE
    || (button == BUTTON_TUNE  && radio_obj.tune) 
    #endif
    #ifdef USE_LOCK
    || (button == BUTTON_LOCK  && radio_obj.lock)
    #endif
    || (button == BUTTON_RIT   && radio_obj.rit)
    #ifdef USE_SPOT
    || (button == BUTTON_SPOT  && radio_obj.spot)
    #endif
    || (button == BUTTON_CWS   && radio_obj.rotary_mode == RM_CWS)
    #ifdef USE_IF_SHIFT
    || (button == BUTTON_IF_SHIFT && radio_obj.rotary_mode == RM_IF_SHIFT)
    #endif
    #ifdef USE_RF_SHIFT
    || (button == BUTTON_RF_SHIFT && radio_obj.rotary_mode == RM_RF_SHIFT)
    #endif
    #ifdef USE_RF_SHIFT_STEP
    || (button == BUTTON_RF_SHIFT && radio_obj.rf_attn_step)
    #endif
    #ifdef USE_ATTN
    || (button == BUTTON_ATTN  && (radio_obj.vfo_data[radio_obj.using_vfo_b].attn))
    #endif
    #ifdef USE_TUNE
    || (button == BUTTON_TUNE  && radio_obj.tune)
    #endif
    || (button == BUTTON_RIT   && radio_obj.rotary_mode == RM_RIT)) {
    highlight = !highlight;
  }
  loop_master::highlight_clear_button(button, highlight);
};