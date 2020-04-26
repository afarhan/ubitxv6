#pragma once

#include <stdint.h>

enum ButtonStatus_e : uint8_t {
  Stateless,
  Inactive,
  Active
};

struct Button {
  const char* text;//nullptr if text_override should be used
  void (*text_override)(char* text_out, const uint16_t max_text_size);//nullptr if text should be used
  ButtonStatus_e (*status)();//Used for coloring and morse menu
  void (*on_select)();//Action to take when selected
  char morse;
};

struct ButtonGrid_t {
  int16_t top_left_x;
  int16_t top_left_y;
  uint16_t button_width;
  uint16_t button_height;
  uint16_t button_pitch_x;
  uint16_t button_pitch_y;
  uint8_t num_button_rows;
  uint8_t num_button_cols;
  const Button* const* buttons_P;//Expected to be in progmem
};

void drawButton(const ButtonGrid_t *const button_grid_P,
                const Button *const button_P);
void drawButtonGrid(const ButtonGrid_t *const button_grid_P);
