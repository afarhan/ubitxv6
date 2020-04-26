#pragma once

#include <stdint.h>

#include "button_grid.h"

extern const ButtonGrid_t mainMenuVfoGrid;
extern const ButtonGrid_t mainMenuGrid;
extern const Button bVfoA;
extern const Button bVfoB;

void updateBandButtons(const uint32_t old_freq);
void updateSidebandButtons();
