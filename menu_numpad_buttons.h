#pragma once

#include <stdint.h>

#include "button.h"
#include "button_press_e.h"

extern const Button* const numpadMenuButtons[];
extern const uint8_t NUMPAD_MENU_NUM_BUTTONS;

extern const uint32_t NUMPAD_MENU_EXIT_FREQ;
extern uint32_t numpadMenuFrequency;
extern ButtonPress_e numpadSelectionMode;//NotPressed means exit menu. Other press types are consumed by selectors
