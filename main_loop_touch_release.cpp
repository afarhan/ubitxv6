#include "main_loop.h"
#include "main_buttons.h"
#include "band.h"
#include "keypad.h"
#include "setup.h"
#include "texts.h"
#include "radio.h"
#include "touchscreen_calibration.h"

void main_loop::touch_release(const byte button) {

  switch (button) {
    #if defined USE_HAM_BAND || defined USE_BC_BAND
      case BUTTON_BAND:
        next = &loop_band;
        break;
    #endif 

    case BUTTON_FINP:
      loop_keypad.set_parameters(INPUT_FREQ, radio_obj.set_khz);
      next = &loop_keypad;
      break;
      
    case BUTTON_SETUP:
      next = &loop_setup;
      break;

    case FREQ_DISP_A1:
    case FREQ_DISP_A2:
      radio_obj.set_active_vfob(false);
      break;

    case FREQ_DISP_B1:
    case FREQ_DISP_B2:
      radio_obj.set_active_vfob(true);
      break;

    case BUTTON_MODE:
      radio_obj.toggle_cw();
      break;

    case BUTTON_STEP: 
      radio_obj.inc_step(false);
      break;

    case BUTTON_SPLIT:
      radio_obj.toggle_split();
      break;

    #ifdef USE_TUNE
    case BUTTON_TUNE:
      radio_obj.toggle_tune();
      break;
    #endif

    #ifdef USE_LOCK
    case BUTTON_LOCK:
      radio_obj.toggle_lock();
      break;
    #endif

    case BUTTON_RIT:
      radio_obj.toggle_rit(false);
      break;

    case BUTTON_CWS:
      radio_obj.toggle_cw_speed();
      break;

    #ifdef USE_IF_SHIFT
      case BUTTON_IF_SHIFT:
        radio_obj.toggle_if_shift(false);
        break;
    #endif

    #ifdef USE_RF_SHIFT
      case BUTTON_RF_SHIFT:
        radio_obj.toggle_rf_shift(false);
        break;
    #endif

    #ifdef USE_RF_SHIFT_STEP
      case BUTTON_RF_SHIFT:
        radio_obj.inc_rf_shift_step(false);
        break;
    #endif

    #ifdef USE_ATTN
      case BUTTON_ATTN:
        radio_obj.toggle_attn();
        break;
    #endif
    
    #ifdef USE_SPOT
      case BUTTON_SPOT:
        radio_obj.toggle_spot();
        break;
      #endif

    #ifdef USE_MAN_SAVE  
      case BUTTON_SAVE:
        radio_obj.write_vfoa();
        radio_obj.write_vfob();
        break;
    #endif

    #ifdef USE_MEMORY
    case BUTTON_M1:
      radio_obj.read_memory_eeprom(0);
      break;
    case BUTTON_M2:
      radio_obj.read_memory_eeprom(1);
      break;
    case BUTTON_M3:
      radio_obj.read_memory_eeprom(2);
      break;
    case BUTTON_M4:
      radio_obj.read_memory_eeprom(3);
      break;
    case BUTTON_M5:
      radio_obj.read_memory_eeprom(4);
      break;
    #endif

    case SWITCH:
      if (radio_obj.in_cw_speed) radio_obj.toggle_cw_speed();
      #ifdef USE_IF_SHIFT
        else if (radio_obj.in_if_shift) radio_obj.toggle_if_shift(false);
      #endif
        else if (radio_obj.rit) radio_obj.toggle_rit(false);
      #ifdef USE_SPOT
        else if (radio_obj.spot) radio_obj.toggle_spot();
      #endif
      else next = &loop_ts_calibration;
    break;
  } 
}