//This file is only meant to be included by menu_main.cpp

#include <avr/pgmspace.h>

#include "button.h"
#include "settings.h"

static const unsigned int LAYOUT_VFO_LABEL_X = 0;
static const unsigned int LAYOUT_VFO_LABEL_Y = 10;
static const unsigned int LAYOUT_VFO_LABEL_WIDTH = 159;
static const unsigned int LAYOUT_VFO_LABEL_HEIGHT = 36;
static const unsigned int LAYOUT_VFO_LABEL_PITCH_X = 160;

static const unsigned int LAYOUT_MODE_TEXT_X = 0;
static const unsigned int LAYOUT_MODE_TEXT_Y = LAYOUT_VFO_LABEL_Y + LAYOUT_VFO_LABEL_HEIGHT + 1;
static const unsigned int LAYOUT_MODE_TEXT_WIDTH = 320;
static const unsigned int LAYOUT_MODE_TEXT_HEIGHT = 36;

static const unsigned int LAYOUT_BUTTON_X = 2;
static const unsigned int LAYOUT_BUTTON_Y = LAYOUT_MODE_TEXT_Y + LAYOUT_MODE_TEXT_HEIGHT + 1;
static const unsigned int LAYOUT_BUTTON_WIDTH = 60;
static const unsigned int LAYOUT_BUTTON_HEIGHT = 36;
static const unsigned int LAYOUT_BUTTON_PITCH_X = 64;
static const unsigned int LAYOUT_BUTTON_PITCH_Y = 40;

static const unsigned int LAYOUT_CW_TEXT_X = 0;
static const unsigned int LAYOUT_CW_TEXT_Y = LAYOUT_BUTTON_Y + 3*LAYOUT_BUTTON_PITCH_Y + 1;
static const unsigned int LAYOUT_CW_TEXT_WIDTH = 220;
static const unsigned int LAYOUT_CW_TEXT_HEIGHT = 36;

static const unsigned int LAYOUT_VERSION_TEXT_X = LAYOUT_CW_TEXT_X + LAYOUT_CW_TEXT_WIDTH + 1;
static const unsigned int LAYOUT_VERSION_TEXT_Y = LAYOUT_CW_TEXT_Y;
static const unsigned int LAYOUT_VERSION_TEXT_WIDTH = 320 - LAYOUT_CW_TEXT_WIDTH - 1;
static const unsigned int LAYOUT_VERSION_TEXT_HEIGHT = LAYOUT_CW_TEXT_HEIGHT;

static const unsigned int LAYOUT_TX_X = 280;
static const unsigned int LAYOUT_TX_Y = LAYOUT_MODE_TEXT_Y;
static const unsigned int LAYOUT_TX_WIDTH = 40;
static const unsigned int LAYOUT_TX_HEIGHT = 36;

enum btn_set_e {
  BUTTON_VFOA,
  BUTTON_VFOB,
  BUTTON_RIT,
  BUTTON_USB,
  BUTTON_LSB,
  BUTTON_CW,
  BUTTON_SPL,
  BUTTON_80,
  BUTTON_40,
  BUTTON_30,
  BUTTON_20,
  BUTTON_17,
  BUTTON_15,
  BUTTON_10,
  BUTTON_BLANK_1,
  BUTTON_MNU,
  BUTTON_FRQ,
  BUTTON_TOTAL
};

void msVfoA(int8_t* val_out){
  *val_out = (Vfo_e::VFO_A == globalSettings.activeVfo) ? 1 : -1;
}
void msVfoB(int8_t* val_out){
  *val_out = (Vfo_e::VFO_B == globalSettings.activeVfo) ? 1 : -1;
}
void msRit(int8_t* val_out){
  *val_out = globalSettings.ritOn ? 1 : -1;
}
void msUsb(int8_t* val_out){
  *val_out = (VfoMode_e::VFO_MODE_USB == GetActiveVfoMode()) ? 1 : -1;
}
void msLsb(int8_t* val_out){
  *val_out = (VfoMode_e::VFO_MODE_LSB == GetActiveVfoMode()) ? 1 : -1;
}
void msCw(int8_t* val_out){
  *val_out = (TuningMode_e::TUNE_CW == globalSettings.tuningMode) ? 1 : -1;
}
void msSpl(int8_t* val_out){
  *val_out = globalSettings.splitOn ? 1 : -1;
}
void msIgnore(int8_t* val_out){
  *val_out = 0;
}

constexpr Button mainMenuButtons[] PROGMEM = {
  {LAYOUT_VFO_LABEL_X + 0*LAYOUT_VFO_LABEL_PITCH_X, LAYOUT_VFO_LABEL_Y, LAYOUT_VFO_LABEL_WIDTH, LAYOUT_VFO_LABEL_HEIGHT, BUTTON_VFOA, "VFOA", 'A', msVfoA},
  {LAYOUT_VFO_LABEL_X + 1*LAYOUT_VFO_LABEL_PITCH_X, LAYOUT_VFO_LABEL_Y, LAYOUT_VFO_LABEL_WIDTH, LAYOUT_VFO_LABEL_HEIGHT, BUTTON_VFOB, "VFOB", 'B', msVfoB},
  
  {LAYOUT_BUTTON_X + 0*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_RIT, "RIT", 'R', msRit},
  {LAYOUT_BUTTON_X + 1*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_USB, "USB", 'U', msUsb},
  {LAYOUT_BUTTON_X + 2*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_LSB, "LSB", 'L', msLsb},
  {LAYOUT_BUTTON_X + 3*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_CW ,  "CW", 'C', msCw},
  {LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_SPL, "SPL", 'S', msSpl},

  {LAYOUT_BUTTON_X + 0*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_80, "80", '8', msIgnore},
  {LAYOUT_BUTTON_X + 1*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_40, "40", '4', msIgnore},
  {LAYOUT_BUTTON_X + 2*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_30, "30", '3', msIgnore},
  {LAYOUT_BUTTON_X + 3*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_20, "20", '2', msIgnore},
  {LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_17, "17", '7', msIgnore},

  {LAYOUT_BUTTON_X + 0*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_15 ,  "15", '5', msIgnore},
  {LAYOUT_BUTTON_X + 1*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_10 ,  "10", '1', msIgnore},
  {LAYOUT_BUTTON_X + 2*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_BLANK_1, "", '\0', msIgnore},
  {LAYOUT_BUTTON_X + 3*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_MNU, "\x7F", 'M', msIgnore},
  {LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_FRQ, "FRQ", 'F', msIgnore},
};

static constexpr uint8_t MAIN_MENU_NUM_BUTTONS = sizeof(mainMenuButtons) / sizeof(mainMenuButtons[0]);
