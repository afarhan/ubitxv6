#pragma once

#include <stdint.h>

enum ButtonStatus_e : uint8_t {
  Stateless,
  Inactive,
  Active
};

struct Button {
  int16_t x, y, w, h;
  const char* text;//nullptr if text_override should be used
  void (*text_override)(char* text_out, uint16_t max_text_size);//nullptr if text should be used
  ButtonStatus_e (*status)();//Used for coloring and morse menu
  void (*on_select)();//Action to take when selected
  char morse;
};