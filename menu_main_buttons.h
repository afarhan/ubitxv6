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

void toVfo(char* text_out, const uint16_t max_text_size, const Vfo_e vfo)
{
  if(max_text_size < 2){
    return;//Can't do much with that space
  }
  if(max_text_size < (3+10+1)){
    //Give an indicator that's debuggable
    text_out[0] = 'X';
    text_out[1] = '\0';
    return;
  }

  //Normal operation
  if (globalSettings.splitOn){
    if(vfo == globalSettings.activeVfo){
      text_out[0] = 'R';
    }
    else{
      text_out[0] = 'T';
    }
  }
  else{
    if(VFO_A == vfo){
      text_out[0] = 'A';
    }
    else if(VFO_B == vfo){
      text_out[0] = 'B';
    }
    else{
      text_out[0] = '?';
    }
  }
  text_out[1] = ':';
  text_out[2] = ' ';
  if(VFO_A == vfo){
    formatFreq(globalSettings.vfoA.frequency, text_out+3, max_text_size-3, 10);
  }
  else if(VFO_B == vfo){
    formatFreq(globalSettings.vfoB.frequency, text_out+3, max_text_size-3, 10);
  }
  else{
    text_out[3] = '?';
    text_out[4] = '\0';
  }
}

void bsVfo(const Vfo_e vfo){
  (vfo == globalSettings.activeVfo) ? ButtonStatus_e::Active : ButtonStatus_e::Inactive;
}

void osVfo(const Vfo_e vfo){
  globalSettings.activeVfo = vfo;
}

void toVfoA(char* text_out, const uint16_t max_text_size){
  toVfo(text_out,max_text_size,Vfo_e::VFO_A);
}
ButtonStatus_e bsVfoA(){
  bsVfo(Vfo_e::VFO_A);
}
void osVfoA(){
  osVfo(Vfo_e::VFO_A);
}
constexpr Button bVfoA = {
  LAYOUT_VFO_LABEL_X + 0*LAYOUT_VFO_LABEL_PITCH_X,
  LAYOUT_VFO_LABEL_Y,
  LAYOUT_VFO_LABEL_WIDTH,
  LAYOUT_VFO_LABEL_HEIGHT,
  nullptr,
  toVfoA,
  bsVfoA,
  osVfoA,
  'A'
};

void toVfoB(char* text_out, const uint16_t max_text_size){
  toVfo(text_out,max_text_size,Vfo_e::VFO_B);
}
ButtonStatus_e bsVfoB(){
  bsVfo(Vfo_e::VFO_B);
}
void osVfoB(){
  osVfo(Vfo_e::VFO_B);
}
constexpr Button bVfoB = {
  LAYOUT_VFO_LABEL_X + 1*LAYOUT_VFO_LABEL_PITCH_X,
  LAYOUT_VFO_LABEL_Y,
  LAYOUT_VFO_LABEL_WIDTH,
  LAYOUT_VFO_LABEL_HEIGHT,
  nullptr,
  toVfoB,
  bsVfoB,
  osVfoB,
  'B'
};

constexpr char txtRit [] PROGMEM = "RIT";
ButtonStatus_e bsRit(){
  *val_out = globalSettings.ritOn ? 1 : -1;
}
void osRit(){
  Button button;
  if(!globalSettings.ritOn){
    globalSettings.ritOn = true;
    globalSettings.ritFrequency = GetActiveVfoFreq();

    strncpy_P(b,(const char*)F("TX: "),sizeof(b));
    formatFreq(globalSettings.ritFrequency, b+3, sizeof(b)-strlen(c));
    if (VFO_A == globalSettings.activeVfo){
      displayText(b, LAYOUT_VFO_LABEL_X + 0*LAYOUT_VFO_LABEL_PITCH_X, LAYOUT_MODE_TEXT_Y, LAYOUT_VFO_LABEL_WIDTH, LAYOUT_MODE_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
    }
    else{
      displayText(b, LAYOUT_VFO_LABEL_X + 1*LAYOUT_VFO_LABEL_PITCH_X, LAYOUT_MODE_TEXT_Y, LAYOUT_VFO_LABEL_WIDTH, LAYOUT_MODE_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
    }
  }
  else{
    globalSettings.ritOn = false;
    setFrequency(globalSettings.ritFrequency);

    displayFillrect(LAYOUT_MODE_TEXT_X,LAYOUT_MODE_TEXT_Y,LAYOUT_MODE_TEXT_WIDTH,LAYOUT_MODE_TEXT_HEIGHT, COLOR_BACKGROUND);
    if(Vfo_e::VFO_A == globalSettings.activeVfo){
      memcpy_P(&button,bVfoA,sizeof(button));
      drawButton(&button);
    }
    else{
      memcpy_P(&button,bVfoB,sizeof(button));
      drawButton(&button);
    }
  }
  
  memcpy_P(&button,bRit,sizeof(button));
  drawButton(button);
}
constexpr Button bRit = {
  LAYOUT_BUTTON_X + 0*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txtRit,
  nullptr,
  bsRit,
  osRit,
  'R'
};


ButtonStatus_e bsUsb(){
  *val_out = (VfoMode_e::VFO_MODE_USB == GetActiveVfoMode()) ? 1 : -1;
}
ButtonStatus_e bsLsb(){
  *val_out = (VfoMode_e::VFO_MODE_LSB == GetActiveVfoMode()) ? 1 : -1;
}
ButtonStatus_e bsCw(){
  *val_out = (TuningMode_e::TUNE_CW == globalSettings.tuningMode) ? 1 : -1;
}
ButtonStatus_e bsSpl(){
  *val_out = globalSettings.splitOn ? 1 : -1;
}
ButtonStatus_e bsIgnore(){
  *val_out = 0;
}

constexpr Button mainMenuButtons[] PROGMEM = {
  bVfoA,
  bVfoB,

  bRit,
  
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
