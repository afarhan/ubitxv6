#pragma once

#include <Arduino.h>
#include "loop_master.h"
#include "defines.h"

class setup_loop : public loop_master {
  virtual void setup(void);
  virtual void touch_release(const byte button);
  virtual bool touch(const byte button);

  virtual void highlight_clear_button(const byte button, bool highlight);
  virtual void button_setup(void);
  virtual void button_text(const byte button);
  virtual void update_keyer_mode(void);
  #ifdef USE_PDL_POL
  virtual void update_paddle_polarity(void);
  #endif
  #ifdef USE_TX_DIS
  virtual void update_tx_disable(void);
  #endif

  virtual void update_cw_pitch(void);
  virtual void update_cw_pitch_display(void);

  virtual void update_cw_delay(void);
  virtual void update_cw_delay_display(void);

  virtual void update_freq_cal(void);
  virtual void update_freq_cal_display(void);

  virtual void update_bfo_cal(void);
  virtual void update_bfo_cal_display(void);

  #ifdef USE_TUNE
  virtual void update_tune_pwr(void);
  virtual void update_tune_pwr_display(void);
  #endif

};

extern setup_loop loop_setup;
