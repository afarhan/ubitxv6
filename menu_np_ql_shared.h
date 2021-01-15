#pragma once

#include "button.h"
#include "menu.h"

MenuReturn_e runNpQlShared(const ButtonPress_e tuner_button,
                           const ButtonPress_e touch_button,
                           const Point touch_point,
                           const int16_t knob,
                           int16_t *const menuSelectedItemRaw,
                           const Button *const *const menu_buttons,
                           const uint8_t menu_num_buttons,
                           ButtonPress_e *const selection_mode);

