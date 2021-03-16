#include "main_loop.h"
#include "main_buttons.h"
#include "display.h"
#include "radio.h"

void main_loop::hold_release(const byte button) {
  switch (button) {
    case BUTTON_RIT:
      radio_obj.toggle_rit(true);
      break;

    #ifdef USE_IF_SHIFT
      case BUTTON_IF_SHIFT:
        radio_obj.toggle_if_shift(true);
        break;
    #endif

    #ifdef USE_RF_SHIFT
      case BUTTON_RF_SHIFT:
        radio_obj.toggle_rf_shift(true);
        break;
    #endif

    #ifdef USE_RF_SHIFT_STEP
      case BUTTON_RF_SHIFT:
        radio_obj.inc_rf_shift_step(true);
        break;
    #endif

    case FREQ_DISP_A1:
    case FREQ_DISP_A2:
      radio_obj.eq_vfo(false);
      break;
      
    case FREQ_DISP_B1:
    case FREQ_DISP_B2:
      radio_obj.eq_vfo(true);
      break;

    case BUTTON_STEP:
      radio_obj.inc_step(true);
      break;

    case BUTTON_MODE:
      radio_obj.toggle_usb();
      break;

    #ifdef USE_MEMORY
    case BUTTON_M1:
      radio_obj.write_memory_eeprom(0);
      break;
    case BUTTON_M2:
      radio_obj.write_memory_eeprom(1);
      break;
    case BUTTON_M3:
      radio_obj.write_memory_eeprom(2);
      break;
    case BUTTON_M4:
      radio_obj.write_memory_eeprom(3);
      break;
    case BUTTON_M5:
      radio_obj.write_memory_eeprom(4);
      break;
    #endif
      
    case SWITCH:
      #ifdef USE_IF_SHIFT
      if (radio_obj.in_if_shift) radio_obj.toggle_if_shift(true);
      #endif
      if (radio_obj.rit) radio_obj.toggle_rit(true);
    break;
      
  }
}