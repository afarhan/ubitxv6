#pragma once


#include <Arduino.h>
#include "loop_master.h"

class keypad_loop : public loop_master {
  static long value;
  static bool (*return_value)(const long);
  virtual void setup(void);
  virtual void touch_release(const byte button);
  virtual void button_setup(void);
  virtual void button_text(const byte button);
  void do_digit(const byte digit);
  public:
    void set_parameters(const char *title, bool (*return_value)(const long));
};

extern keypad_loop loop_keypad;
