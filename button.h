#pragma once

#include <stdint.h>

struct Button {
  int x, y, w, h;
  unsigned int id;
  char text[5];
  char morse;
  void (*morse_status)(int8_t* val_out);//-1 if a low tone should play, +1 if a high tone should play
};