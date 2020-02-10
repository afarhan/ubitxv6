#pragma once

#include "point.h"

void initTouch();

//Returns true if touched, false otherwise
bool readTouch(Point *const touch_point_out);

//Applies the touch calibration the point passed in
void scaleTouch(Point *const touch_point_in_out);