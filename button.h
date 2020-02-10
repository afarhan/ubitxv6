#pragma once

#include <stdint.h>

enum ButtonStatus_e : uint8_t {
  Stateless,
  Inactive,
  Active
};

struct Button {
  int16_t x, y, w, h;
  const char* text;
  void (*text_override)(char* text_out, uint16_t max_text_size);
  char morse;
  ButtonStatus_e (*status)();
  void (*on_select)();//Action to take when selected
};