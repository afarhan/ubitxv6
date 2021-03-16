#pragma once

#include "loop_master.h"
#include "radio.h"
#include "defines.h"

#if defined USE_HAM_BAND || defined USE_BC_BAND

class band_loop : public loop_master {
  virtual void touch_release(const byte button);
  virtual void button_setup(void);
  virtual void button_text(const byte button);
  void set_band(const RADIO_BAND band);
  public:
    void set_parameters(const char *title, bool (*return_value)(const long));
};

extern band_loop loop_band;
#endif