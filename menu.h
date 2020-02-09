#pragma once

#include <stdint.h>
#include "button.h"
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
};

struct Menu_t {
  MenuReturn_e (*const runMenu)(const ButtonPress_e tuner_button,
                                const ButtonPress_e touch_button,
                                const Point touch_point,
                                const int16_t knob);
  Menu_t* active_submenu;
};

static const uint8_t MENU_KNOB_COUNTS_PER_ITEM = 10;

//Returns true if submenu was run, false otherwise
bool runSubmenu(Menu_t* current_menu,
                void(*const redraw_callback)(),
                const ButtonPress_e tuner_button,
                const ButtonPress_e touch_button,
                const Point touch_point,
                const int16_t knob);

void movePuck(const Button *const b_old,
              const Button *const b_new);
