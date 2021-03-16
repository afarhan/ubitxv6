#include "radio.h"

ROTARY_MODE radio::rotary_mode;

void radio::rotary(const bool inc) {
  switch (rotary_mode) {
    case RM_VFO:
      adj_frequency(inc);
      break;

    case RM_RIT:
      adj_rit(inc);
      break;

    case RM_CWS:
      radio_obj.adj_cw_speed(inc);
      break;

    #ifdef USE_IF_SHIFT
      case RM_IF_SHIFT:
        radio_obj.adj_if_shift(inc);
        break;
    #endif

    case RM_CWP:
      radio_obj.adj_cw_pitch(inc);
      break;

    case RM_BFO:
      radio_obj.adj_bfo(inc);
      break;

    case RM_FREQ:
      radio_obj.adj_master_cal(inc);
      break;

    case RM_DELAY:
      radio_obj.adj_cw_delay(inc);
      break;

    #ifdef USE_TUNE
      case RM_TUNE_PWR:
        radio_obj.adj_tune_pwr(inc);
        break;
    #endif

    #ifdef USE_RF_SHIFT
      case RM_RF_SHIFT:
        radio_obj.adj_rf_shift(inc);
        break;
    #endif
  }
}