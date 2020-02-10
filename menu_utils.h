#pragma once

#include "button.h"
#include "menu.h"

//Returns true if submenu was run, false otherwise
bool runSubmenu(Menu_t* current_menu,
                void(*const redraw_callback)(),
                const ButtonPress_e tuner_button,
                const ButtonPress_e touch_button,
                const Point touch_point,
                const int16_t knob);

//Returns true if button was found, false otherwise
bool findPressedButton(const Button* const* buttons,
                       const uint8_t num_buttons,
                       Button *const button_out,
                       const Point touch_point);

enum MorsePlaybackType_e : uint8_t {
  PlayChar,
  PlayText
};
void initSelector(int16_t *const raw_select_val_in_out,
                  const Button* const* buttons,
                  const uint8_t num_buttons,
                  const MorsePlaybackType_e);

void adjustSelector(int16_t *const raw_select_val_in_out,
                    int16_t knob,
                    const Button* const* buttons,
                    const uint8_t num_buttons,
                    const MorsePlaybackType_e);

void endSelector(const Button *const button);
