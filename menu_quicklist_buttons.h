#pragma once

#include <stdint.h>

#include "button.h"
#include "button_press_e.h"

extern const Button* const quickListMenuButtons[];
extern const uint8_t QUICKLIST_MENU_NUM_BUTTONS;

extern ButtonPress_e quickListSelectionMode;//NotPressed means exit menu. Other press types are consumed by selectors
