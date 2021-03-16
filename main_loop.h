#pragma once

#include <Arduino.h>
#include "loop_master.h"
#include "defines.h"
#include "radio.h"

class main_loop : public loop_master {
  virtual void setup(void);
  virtual void ptt_change(const bool down); 
  virtual void highlight_clear_button(const byte button, bool highlight);
  virtual void button_setup(void);
  virtual void touch_release(const byte button);
  virtual void hold_release(const byte button);
  virtual void button_text(const byte button);
  virtual bool touch(const byte button);

  virtual void update_vfo(const bool vfob);
  virtual void update_mode(void);
  virtual void update_step(void);
  #ifdef USE_ATTN
  virtual void update_attn(void);
  #endif
  #ifdef USE_IF_SHIFT
  virtual void update_if_shift(void);
  virtual void update_if_shift_display(void);
  #endif
  virtual void update_split(void);
  #ifdef USE_TUNE
  virtual void update_tune(void);
  #endif
  #ifdef USE_LOCK
  virtual void update_lock(void);
  #endif
  virtual void update_cw_speed(void);
  virtual void update_cw_speed_display(void);
  #ifdef USE_SPOT
  virtual void update_spot(void);
  #endif
  virtual void update_rit(void);
  #ifdef USE_MEMORY
  virtual void update_memory(const byte which);
  #endif

  #ifdef USE_RF_SHIFT
  virtual void update_rf_shift(void);
  virtual void update_rf_shift_display(void);
  #endif

  #ifdef USE_RF_SHIFT_STEP
  virtual void update_rf_shift_step(void);
  #endif
};

extern main_loop loop_main;
