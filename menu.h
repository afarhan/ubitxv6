#pragma once

#include <stdint.h>
#include "nano_gui.h"//Point

enum MenuReturn_e : uint8_t {
  StillActive,
  ExitedRedraw,
  ExitedNoRedraw
};

enum ButtonPress_e : uint8_t {
  NotPressed,
  ShortPress,
  LongPress
}

struct Menu_t {
  MenuReturn_e (*runMenu)(ButtonPress_e tuner_button,
                          ButtonPress_e touch_button,
                          Point touch_point,
                          int16_t knob);
  Menu_t* active_submenu;
};

static const uint8_t MENU_KNOB_COUNTS_PER_ITEM = 10;

//Returns true if submenu was run, false otherwise
bool runSubmenu(Menu_t* current_menu,
                void(*redraw_callback)(),
                ButtonPress_e tuner_button,
                ButtonPress_e touch_button,
                Point touch_point,
                int16_t knob);