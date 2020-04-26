#include "menu_main_buttons.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <WString.h>//F()

#include "bands.h"
#include "button.h"
#include "callsign.h"
#include "color_theme.h"
#include "menu_main.h"
#include "menu_numpad.h"
#include "morse.h"
#include "nano_gui.h"
#include "scratch_space.h"
#include "settings.h"
#include "setup.h"
#include "tuner.h"
#include "utils.h"
#include "version.h"

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

void drawTx()
{
  if(globalSettings.txActive){
    strncpy_P(b,(const char*)F("TX"),sizeof(b));
    displayText(b,LAYOUT_TX_X,LAYOUT_TX_Y,LAYOUT_TX_WIDTH,LAYOUT_TX_HEIGHT,COLOR_ACTIVE_TEXT,COLOR_ACTIVE_BACKGROUND,COLOR_BACKGROUND);
  }
  else{
    displayFillrect(LAYOUT_TX_X,LAYOUT_TX_Y,LAYOUT_TX_WIDTH,LAYOUT_TX_HEIGHT,COLOR_BACKGROUND);
  }
}

void drawVersion()
{
  strncpy_P(b,VERSION_STRING,sizeof(b));
  displayText(b,LAYOUT_VERSION_TEXT_X,LAYOUT_VERSION_TEXT_Y,LAYOUT_VERSION_TEXT_WIDTH,LAYOUT_VERSION_TEXT_HEIGHT,COLOR_VERSION_TEXT,COLOR_BACKGROUND,COLOR_BACKGROUND);
}

void drawCallsign()
{
  strncpy_P(b,CALLSIGN_STRING,sizeof(b));
  displayText(b,LAYOUT_CW_TEXT_X,LAYOUT_CW_TEXT_Y,LAYOUT_CW_TEXT_WIDTH,LAYOUT_CW_TEXT_HEIGHT,COLOR_VERSION_TEXT,COLOR_BACKGROUND,COLOR_BACKGROUND);
}

void toVfoA(char* text_out, const uint16_t max_text_size);
ButtonStatus_e bsVfoA();
void osVfoA();
constexpr Button bVfoA PROGMEM = {
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

void toVfoB(char* text_out, const uint16_t max_text_size);
ButtonStatus_e bsVfoB();
void osVfoB();
constexpr Button bVfoB PROGMEM = {
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
ButtonStatus_e bsRit();
void osRit();
constexpr Button bRit PROGMEM = {
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

constexpr char txtUsb [] PROGMEM = "USB";
ButtonStatus_e bsUsb();
void osUsb();
constexpr Button bUsb PROGMEM = {
  LAYOUT_BUTTON_X + 1*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txtUsb,
  nullptr,
  bsUsb,
  osUsb,
  'U'
};

constexpr char txtLsb [] PROGMEM = "LSB";
ButtonStatus_e bsLsb();
void osLsb();
constexpr Button bLsb PROGMEM = {
  LAYOUT_BUTTON_X + 2*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txtLsb,
  nullptr,
  bsLsb,
  osLsb,
  'L'
};

constexpr char txtCw [] PROGMEM = "CW";
ButtonStatus_e bsCw();
void osCw();
constexpr Button bCw PROGMEM = {
  LAYOUT_BUTTON_X + 3*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txtCw,
  nullptr,
  bsCw,
  osCw,
  'C'
};

constexpr char txtSpl [] PROGMEM = "SPL";
ButtonStatus_e bsSpl();
void osSpl();
constexpr Button bSpl PROGMEM = {
  LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txtSpl,
  nullptr,
  bsSpl,
  osSpl,
  'S'
};

constexpr char txt80 [] PROGMEM = "80";
ButtonStatus_e bs80();
void os80();
constexpr Button b80 PROGMEM = {
  LAYOUT_BUTTON_X + 0*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txt80,
  nullptr,
  bs80,
  os80,
  '8'
};

constexpr char txt40 [] PROGMEM = "40";
ButtonStatus_e bs40();
void os40();
constexpr Button b40 PROGMEM = {
  LAYOUT_BUTTON_X + 1*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txt40,
  nullptr,
  bs40,
  os40,
  '4'
};

constexpr char txt30 [] PROGMEM = "30";
ButtonStatus_e bs30();
void os30();
constexpr Button b30 PROGMEM = {
  LAYOUT_BUTTON_X + 2*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txt30,
  nullptr,
  bs30,
  os30,
  '3'
};

constexpr char txt20 [] PROGMEM = "20";
ButtonStatus_e bs20();
void os20();
constexpr Button b20 PROGMEM = {
  LAYOUT_BUTTON_X + 3*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txt20,
  nullptr,
  bs20,
  os20,
  '2'
};

constexpr char txt17 [] PROGMEM = "17";
ButtonStatus_e bs17();
void os17();
constexpr Button b17 PROGMEM = {
  LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txt17,
  nullptr,
  bs17,
  os17,
  '7'
};

constexpr char txt15 [] PROGMEM = "15";
ButtonStatus_e bs15();
void os15();
constexpr Button b15 PROGMEM = {
  LAYOUT_BUTTON_X + 0*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txt15,
  nullptr,
  bs15,
  os15,
  '5'
};

constexpr char txt10 [] PROGMEM = "10";
ButtonStatus_e bs10();
void os10();
constexpr Button b10 PROGMEM = {
  LAYOUT_BUTTON_X + 1*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txt10,
  nullptr,
  bs10,
  os10,
  '1'
};

constexpr char txtMenu [] PROGMEM = "\x7F";//gear icon
ButtonStatus_e bsIgnore();
void osMenu();
constexpr Button bMenu PROGMEM = {
  LAYOUT_BUTTON_X + 3*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txtMenu,
  nullptr,
  bsIgnore,
  osMenu,
  'M'
};

constexpr char txtNumpad [] PROGMEM = "FRQ";
ButtonStatus_e bsIgnore();
void osNumpad();
constexpr Button bNumpad PROGMEM = {
  LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txtNumpad,
  nullptr,
  bsIgnore,
  osNumpad,
  'F'
};

const Button* const mainMenuButtons [] PROGMEM = {
  &bVfoA,                        &bVfoB,

   &bRit, &bUsb, &bLsb,   &bCw,   &bSpl,
    &b80,  &b40,  &b30,   &b20,    &b17,
    &b15,  &b10,        &bMenu, &bNumpad
};

const uint8_t MAIN_MENU_NUM_BUTTONS = sizeof(mainMenuButtons) / sizeof(mainMenuButtons[0]);

void updateBandButtons(const uint32_t old_freq)
{
  const Button* band_buttons[] = {&b80,&b40,&b30,&b20,&b17,&b15,&b10};
  const uint8_t bands [] =       {  80,  40,  30,  20,  17,  15,  10};
  const uint32_t curr_freq = GetActiveVfoFreq();

  Button button;
  for(uint8_t i = 0; i < sizeof(bands)/sizeof(bands[0]); ++i){
    if(isFreqInBand(old_freq,bands[i]) != isFreqInBand(curr_freq,bands[i])){
      memcpy_P(&button,band_buttons[i],sizeof(button));
      drawButton(&button);
      morseBool(ButtonStatus_e::Active == button.status());
    }
  }
}

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

ButtonStatus_e bsVfo(const Vfo_e vfo){
  return (vfo == globalSettings.activeVfo) ? ButtonStatus_e::Active : ButtonStatus_e::Inactive;
}

void osVfo(const Vfo_e vfo){
  const uint32_t old_freq = GetActiveVfoFreq();
  globalSettings.activeVfo = vfo;
  SaveSettingsToEeprom();

  ltoa(GetActiveVfoFreq(),b,10);
  morseText(b);

  Button button;
  memcpy_P(&button,&bVfoA,sizeof(button));
  drawButton(&button);
  memcpy_P(&button,&bVfoB,sizeof(button));
  drawButton(&button);
  updateBandButtons(old_freq);
  updateSidebandButtons();
}

void toVfoA(char* text_out, const uint16_t max_text_size){
  toVfo(text_out,max_text_size,Vfo_e::VFO_A);
}
ButtonStatus_e bsVfoA(){
  return bsVfo(Vfo_e::VFO_A);
}
void osVfoA(){
  osVfo(Vfo_e::VFO_A);
}

void toVfoB(char* text_out, const uint16_t max_text_size){
  toVfo(text_out,max_text_size,Vfo_e::VFO_B);
}
ButtonStatus_e bsVfoB(){
  return bsVfo(Vfo_e::VFO_B);
}
void osVfoB(){
  osVfo(Vfo_e::VFO_B);
}

ButtonStatus_e bsRit(){
  return globalSettings.ritOn ? ButtonStatus_e::Active : ButtonStatus_e::Inactive;
}
void osRit(){
  Button button;
  if(!globalSettings.ritOn){
    globalSettings.ritOn = true;
    globalSettings.ritFrequency = GetActiveVfoFreq();

    strncpy_P(b,(const char*)F("TX: "),sizeof(b));
    formatFreq(globalSettings.ritFrequency, b + strlen(b), sizeof(b)-strlen(b));
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
      memcpy_P(&button,&bVfoA,sizeof(button));
      drawButton(&button);
    }
    else{
      memcpy_P(&button,&bVfoB,sizeof(button));
      drawButton(&button);
    }
  }
  
  memcpy_P(&button,&bRit,sizeof(button));
  drawButton(&button);
}

void osSidebandMode(VfoMode_e mode){
  SetActiveVfoMode(mode);
  setFrequency(GetActiveVfoFreq());
  SaveSettingsToEeprom();

  Button button;
  memcpy_P(&button,&bUsb,sizeof(button));
  drawButton(&button);
  memcpy_P(&button,&bLsb,sizeof(button));
  drawButton(&button);
}

void updateSidebandButtons()
{
  osSidebandMode(GetActiveVfoMode());
}

ButtonStatus_e bsUsb(){
  return (VfoMode_e::VFO_MODE_USB == GetActiveVfoMode()) ? ButtonStatus_e::Active : ButtonStatus_e::Inactive;
}

void osUsb(){
  osSidebandMode(VfoMode_e::VFO_MODE_USB);
}

ButtonStatus_e bsLsb(){
  return (VfoMode_e::VFO_MODE_LSB == GetActiveVfoMode()) ? ButtonStatus_e::Active : ButtonStatus_e::Inactive;
}

void osLsb(){
  osSidebandMode(VfoMode_e::VFO_MODE_LSB);
}

ButtonStatus_e bsCw(){
  return (TuningMode_e::TUNE_CW == globalSettings.tuningMode) ? ButtonStatus_e::Active : ButtonStatus_e::Inactive;
}

void osCw(){
  if(TuningMode_e::TUNE_CW != globalSettings.tuningMode){
    globalSettings.tuningMode = TuningMode_e::TUNE_CW;
  }
  else{
    globalSettings.tuningMode = TuningMode_e::TUNE_SSB;
  }

  setFrequency(GetActiveVfoFreq());

  Button button;
  memcpy_P(&button,&bCw,sizeof(button));
  drawButton(&button);
}

ButtonStatus_e bsSpl(){
  return globalSettings.splitOn ? ButtonStatus_e::Active : ButtonStatus_e::Inactive;
}

void osSpl(){
  globalSettings.splitOn = !globalSettings.splitOn;

  Button button;
  memcpy_P(&button,&bSpl,sizeof(button));
  drawButton(&button);
  memcpy_P(&button,&bVfoA,sizeof(button));
  drawButton(&button);
  memcpy_P(&button,&bVfoB,sizeof(button));
  drawButton(&button);
}

ButtonStatus_e bsBand(const uint8_t band){
  return isFreqInBand(GetActiveVfoFreq(),band) ? ButtonStatus_e::Active : ButtonStatus_e::Inactive;
}

void osBand(const uint8_t band){
  const uint32_t old_freq = GetActiveVfoFreq();
  SetActiveVfoFreq(getFreqInBand(old_freq,band));
  if(autoSelectSidebandChanged(old_freq)){
    updateSidebandButtons();
  }

  Button button;
  if(Vfo_e::VFO_A == globalSettings.activeVfo){
    memcpy_P(&button,&bVfoA,sizeof(button));
    drawButton(&button);
  }
  else if(Vfo_e::VFO_B == globalSettings.activeVfo){
    memcpy_P(&button,&bVfoB,sizeof(button));
    drawButton(&button);
  }

  updateBandButtons(old_freq);
}

ButtonStatus_e bs80(){
  return bsBand(80);
}

void os80(){
  osBand(80);
}

ButtonStatus_e bs40(){
  return bsBand(40);
}

void os40(){
  osBand(40);
}

ButtonStatus_e bs30(){
  return bsBand(30);
}

void os30(){
  osBand(30);
}

ButtonStatus_e bs20(){
  return bsBand(20);
}

void os20(){
  osBand(20);
}

ButtonStatus_e bs17(){
  return bsBand(17);
}

void os17(){
  osBand(17);
}

ButtonStatus_e bs15(){
  return bsBand(15);
}

void os15(){
  osBand(15);
}

ButtonStatus_e bs10(){
  return bsBand(10);
}

void os10(){
  osBand(10);
}

ButtonStatus_e bsIgnore(){
  return ButtonStatus_e::Stateless;
}

void osMenu(){
  enterSubmenu(setupMenu);
}

void osNumpad(){
  enterSubmenu(numpadMenu);
}
