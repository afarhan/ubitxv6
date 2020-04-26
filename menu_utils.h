#pragma once

#include "button_grid.h"
#include "menu.h"

//Returns true if button was found, false otherwise
bool findPressedButton(const ButtonGrid_t *const button_grid_P,
                       Button *const button_out,
                       const Point touch_point);

enum MorsePlaybackType_e : uint8_t {
  PlayChar,
  PlayText
};
void initSelector(int16_t *const raw_select_val_in_out,
                  const ButtonGrid_t *const button_grid_P,
                  const MorsePlaybackType_e play_type);

void adjustSelector(int16_t *const raw_select_val_in_out,
                    int16_t knob,
                    const ButtonGrid_t *const button_grid_P,
                    const MorsePlaybackType_e play_type);

void endSelector(const int16_t raw_select,
                 const ButtonGrid_t *const button_grid_P);
