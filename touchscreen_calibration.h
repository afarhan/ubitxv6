#pragma once


#include "loop_master.h"

class ts_calibration_loop : public loop_master {
  virtual void init(void);
  virtual void setup(void);
  virtual void calibration_release(void);
  virtual void touch_release(const byte button);

  void set_center(const byte ctr, int &x, int &y);

  byte ctr;
  int x_act[3], y_act[3];
};

extern ts_calibration_loop loop_ts_calibration;
