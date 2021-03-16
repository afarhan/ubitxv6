#pragma once

#include <stdint.h>

struct Point {
	Point(int16_t x, int16_t y, int16_t z) : x(x), y(y), z(z) {};
  Point() {};
  int16_t x;
  int16_t y;
  int16_t z;
};
