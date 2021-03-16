#include "main_loop.h"
#include "main_buttons.h"
#include "radio.h"


bool main_loop::touch(const byte button) {
  switch (button) {
    case BUTTON_RIT:
    case BUTTON_FINP:
    #if defined USE_HAM_BAND || defined USE_BC_BAND
      case BUTTON_BAND:
    #endif
    case BUTTON_SPLIT:
    case FREQ_DISP_A1:
    case FREQ_DISP_A2:
    case FREQ_DISP_B1:
    case FREQ_DISP_B2:
    #ifdef USE_MEMORY
    case BUTTON_M1:
    case BUTTON_M2:
    case BUTTON_M3:
    case BUTTON_M4:
    case BUTTON_M5:
    #endif
      #ifdef USE_LOCK
      return !radio_obj.lock;
      #else
      return true;
      #endif
      break; 
    #ifdef USE_SPOT
    case BUTTON_SPOT:
    #endif
    case BUTTON_CWS:
      return radio_obj.vfo_data[radio_obj.using_vfo_b].cw;
      break; 
  }
  return true;
}
