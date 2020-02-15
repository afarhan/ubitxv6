#pragma once

#include <stdint.h>
#include "point.h"

enum MenuReturn_e : uint8_t {
  StillActive,
  ExitedRedraw,
  ExitedNoRedraw
};

enum ButtonPress_e : uint8_t {
  NotPressed,
  ShortPress,
  LongPress
};

struct Menu_t {
  void (*const initMenu)();//Any initial draw routines or state initialization
  MenuReturn_e (*const runMenu)(const ButtonPress_e tuner_button,
                                const ButtonPress_e touch_button,
                                const Point touch_point,
                                const int16_t knob);
  Menu_t* active_submenu;
};

static const uint8_t MENU_KNOB_COUNTS_PER_ITEM = 10;

void runActiveMenu(const ButtonPress_e tuner_button,
                   const ButtonPress_e touch_button,
                   const Point touch_point,
                   const int16_t knob);

void enterSubmenu(Menu_t *const submenu);
