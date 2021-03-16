#include "main_loop.h"
#include "main_buttons.h"
#include "str_buffer.h"
#include "texts.h"
#include "display.h"
#include "radio.h"

void main_loop::button_text(const byte button) {
  switch (button) {
    #if defined USE_HAM_BAND || defined USE_BC_BAND
      case BUTTON_BAND:
        strcpy_P(str_buffer1, BAND);
        break;
    #endif
    case BUTTON_RIT:
      strcpy_P(str_buffer1, RIT);
      break;
    case BUTTON_SPLIT:
      strcpy_P(str_buffer1, SPLIT);
      break;
    case BUTTON_MODE:
      strcpy_P(str_buffer1, radio_obj.mode_name());
      break;
    case BUTTON_STEP:
      strcpy_P(str_buffer1, radio_obj.step_name());
      break;
    case BUTTON_FINP:
      strcpy_P(str_buffer1, FINP);
      break;
    case BUTTON_CWS:
      strcpy_P(str_buffer1, CWS);
      disp.int_to_str_buffer(str_buffer2, radio_obj.cw_speed);
      break;
    #ifdef USE_TUNE
      case BUTTON_TUNE:
        strcpy_P(str_buffer1, TUNE);
        break;
      #endif
    #ifdef USE_LOCK
      case BUTTON_LOCK:
        strcpy_P(str_buffer1, LOCK);
        break;
    #endif
    case BUTTON_SETUP:
      strcpy_P(str_buffer1, SETUP);
      break;
    #ifdef USE_IF_SHIFT
      case BUTTON_IF_SHIFT:
        strcpy_P(str_buffer1, IF);
        strcpy_P(str_buffer2, SHIFT);
        break;
    #endif
    #ifdef USE_RF_SHIFT
      case BUTTON_RF_SHIFT:
        strcpy_P(str_buffer1, RF);
        strcpy_P(str_buffer2, ATTN);
        break;
    #endif
    #ifdef USE_RF_SHIFT_STEP
      case BUTTON_RF_SHIFT:
        strcpy_P(str_buffer1, RF);
        disp.int_to_str_buffer(str_buffer2, radio_obj.rf_attn_step * 10);
        break;
    #endif
    #ifdef USE_SPOT
      case BUTTON_SPOT:
        strcpy_P(str_buffer1, SPOT);
        break;
    #endif
    #ifdef USE_ATTN
      case BUTTON_ATTN:      
        strcpy_P(str_buffer1, ATTN);
        break;
    #endif
    #ifdef USE_MAN_SAVE
      case BUTTON_SAVE:
        strcpy_P(str_buffer1, SAVE);
        strcpy_P(str_buffer2, VFO);
        break;
    #endif
    #ifdef USE_MEMORY
    case BUTTON_M1:
      disp.mem_button(1);
      break;
    case BUTTON_M2:
      disp.mem_button(2);
      break;
    case BUTTON_M3:
      disp.mem_button(3);
      break;
    case BUTTON_M4:
      disp.mem_button(4);
      break;
    case BUTTON_M5:
      disp.mem_button(5);
      break;
    #endif
  }
}