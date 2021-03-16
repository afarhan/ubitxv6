#include "radio.h"

radio radio_obj;

// storage
long radio::calibration;
long radio::usbCarrier;
long radio::sidetone_frequency;
int radio::cw_millis;
int radio::slope_x;
int radio::slope_y;
int radio::offset_x;
int radio::offset_y;
int radio::cw_delaytime;
KEYER_TYPE radio::cw_key_type;

byte radio::chan_60m = 0;

#ifdef USE_TX_DIS
bool radio::tx_enable = true;
#endif

VFO_DATA radio::vfo_data[2];

uint16_t radio::step_amount;
int16_t radio::rit_amount;

bool radio::rit;
#ifdef USE_IF_SHIFT
bool radio::in_if_shift;
long radio::if_shift_freq;
#endif
bool radio::split;
bool radio::using_vfo_b;
#ifdef USE_TUNE
bool radio::tune;
byte radio::tune_pwr = 1;
bool radio::in_tune_pwr;
#endif
bool radio::ptt;
#ifdef USE_PDL_POL
bool radio::paddle_reverse;
#endif
bool radio::in_tx;
byte radio::cw_speed;
bool radio::cw_tx;
#ifdef USE_SPOT
bool radio::spot;
#endif
#ifdef USE_LOCK
bool radio::lock;
#endif
bool radio::in_bfo_cal;
bool radio::in_cw_delay;
bool radio::in_cw_pitch;
bool radio::in_cw_speed;
bool radio::in_freq_cal;

#ifdef USE_RF_SHIFT
  bool radio::in_rf_shift;
  long radio::rf_shift_freq = firstIF;
#endif
#ifdef USE_RF_SHIFT_STEP
  byte radio::rf_attn_step;
  long radio::rf_shift_freq = firstIF;
#endif

#ifdef USE_AUTO_SAVE
  VFO_AUTOSAVE radio::vfo_autosave[2];
#endif

