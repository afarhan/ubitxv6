#pragma once

#include <stdint.h>

#include "button.h"

extern const Button* const mainMenuButtons[];
extern const uint8_t MAIN_MENU_NUM_BUTTONS;

extern const Button bVfoA;
extern const Button bVfoB;
void updateBandButtons(const uint32_t old_freq);
void updateSidebandButtons();
void drawTx();
void drawVersion();
void drawCallsign();
