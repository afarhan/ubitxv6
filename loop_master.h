#pragma once

#include <Arduino.h>
#include "defines.h"
#include "touch.h"
#include "heartbeat.h"
#include "rotary.h"

const byte SWITCH = 255;

#define BUTTON_BIT(bit) (1UL << (bit))

class loop_master {
  friend void setup(void);
  friend void loop(void);
  friend class radio;

  virtual void init(void) {};  // opportunity for descendent class to override setup

  // methods that can be overridden
  virtual void setup(void) {};
  virtual void hb_125Hz(void) {};
  virtual void hb_1Hz(void) {};
  virtual void ptt_change(const bool) {};
  virtual void touch_release(const byte) {};
  virtual void hold_release(const byte) {};
  virtual void calibration_release(void) {};
  virtual void button_setup(void) {};
  virtual void button_text(const byte) {};
  virtual bool touch(const byte) {return true;};

  // display update methods
  virtual void update_mode(void) {};
  virtual void update_vfo(const bool vfob) {};
  virtual void update_info(void) {};
  #ifdef USE_ATTN
  virtual void update_attn(void) {};
  #endif
  virtual void update_step(void) {};
  virtual void update_split(void) {};
  #ifdef USE_TUNE
  virtual void update_tune(void) {};
  #endif
  #ifdef USE_LOCK
  virtual void update_lock(void) {};
  #endif
  virtual void update_rit(void) {};
  #ifdef USE_SPOT
  virtual void update_spot(void) {};
  #endif

  #ifdef USE_TX_DIS
  virtual void update_tx_disable(void) {};
  #endif
  virtual void update_keyer_mode(void) {};
  #ifdef USE_PDL_POL
  virtual void update_paddle_polarity(void) {};
  #endif

  virtual void update_cw_pitch(void) {};
  virtual void update_cw_pitch_display(void) {};

  virtual void update_cw_speed(void) {};
  virtual void update_cw_speed_display(void) {};

  #ifdef USE_IF_SHIFT
  virtual void update_if_shift(void) {};
  virtual void update_if_shift_display(void) {};
  #endif

  virtual void update_cw_delay(void) {};
  virtual void update_cw_delay_display(void) {};

  virtual void update_freq_cal(void) {};
  virtual void update_freq_cal_display(void) {};

  virtual void update_bfo_cal(void) {};
  virtual void update_bfo_cal_display(void) {};

  virtual void update_tune_pwr(void) {};
  virtual void update_tune_pwr_display(void) {};

  #ifdef USE_MEMORY
  virtual void update_memory(const byte which) {};
  #endif

  #ifdef USE_RF_SHIFT
  virtual void update_rf_shift(void) {};
  virtual void update_rf_shift_display(void) {};
  #endif

  #ifdef USE_RF_SHIFT_STEP
  virtual void update_rf_shift_step(void) {};
  #endif

  virtual void loop(void);
  void make_active(void);
  static unsigned long touch_millis;

  void init_screen(void);
  static void do_loop(void);

  protected:
    static loop_master *next;
    unsigned long draw_buttons, all_buttons, textsize1_buttons, twoline_buttons;

    virtual void highlight_clear_button(const byte button, bool highlight);
    Point p;  // calibrated / uncalibrated touchscreen data
    bool touchscreen_buttons;    
      
  public:
    static loop_master *active;
    static bool need_beep;
};