#pragma once

#include "point.h"

void initTouch(void);

//Returns true if touched, false otherwise
bool readTouch(Point &point);

void scaleTouch(Point &p);