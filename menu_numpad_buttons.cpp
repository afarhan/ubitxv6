#include "menu_numpad_buttons.h"

#include <avr/pgmspace.h>
#include <WString.h>//F()

#include "color_theme.h"
#include "nano_gui.h"
#include "scratch_space.h"
#include "settings.h"
#include "tuner.h"//setFrequency
#include "utils.h"

static const unsigned int LAYOUT_MODE_TEXT_X = 60;
static const unsigned int LAYOUT_MODE_TEXT_Y = 47;
static const unsigned int LAYOUT_MODE_TEXT_WIDTH = 140;
static const unsigned int LAYOUT_MODE_TEXT_HEIGHT = 36;

static const unsigned int LAYOUT_BUTTON_X = 2;
static const unsigned int LAYOUT_BUTTON_Y = LAYOUT_MODE_TEXT_Y + LAYOUT_MODE_TEXT_HEIGHT + 1;
static const unsigned int LAYOUT_BUTTON_WIDTH = 60;
static const unsigned int LAYOUT_BUTTON_HEIGHT = 36;
static const unsigned int LAYOUT_BUTTON_PITCH_X = 64;
static const unsigned int LAYOUT_BUTTON_PITCH_Y = 40;

const uint32_t NUMPAD_MENU_EXIT_FREQ = -1;
uint32_t numpadMenuFrequency;

#define D_STRINGIFY(x) #x
#define D_STRING(x) D_STRINGIFY(x)
#define NUMBER_BUTTON_GENERATE(number) \
          constexpr char txt##number [] PROGMEM = "" D_STRING(number);\
          ButtonStatus_e bs##number();\
          void os##number();\
          constexpr Button b##number PROGMEM = {\
            txt##number,\
            nullptr,\
            bsNumpad,\
            os##number,\
            '0'+number\
          }

ButtonStatus_e bsNumpad(void){
  return ButtonStatus_e::Stateless;
}

NUMBER_BUTTON_GENERATE(1);
NUMBER_BUTTON_GENERATE(2);
NUMBER_BUTTON_GENERATE(3);
NUMBER_BUTTON_GENERATE(4);
NUMBER_BUTTON_GENERATE(5);
NUMBER_BUTTON_GENERATE(6);
NUMBER_BUTTON_GENERATE(7);
NUMBER_BUTTON_GENERATE(8);
NUMBER_BUTTON_GENERATE(9);
NUMBER_BUTTON_GENERATE(0);

constexpr char txtOk [] PROGMEM = "OK";
void osOk();
constexpr Button bOk PROGMEM = {
  txtOk,
  nullptr,
  bsNumpad,
  osOk,
  'K'
};

constexpr char txtBackspace [] PROGMEM = "<-";
void osBackspace();
constexpr Button bBackspace PROGMEM = {
  txtBackspace,
  nullptr,
  bsNumpad,
  osBackspace,
  'B'
};

constexpr char txtCancel [] PROGMEM = "Can";
void osCancel();
constexpr Button bCancel PROGMEM = {
  txtCancel,
  nullptr,
  bsNumpad,
  osCancel,
  'C'
};

//Declare in menu select order, not graphical order
const Button* const numpadMenuButtons [] PROGMEM = {
  &b1, &b2, &b3, nullptr,        &bOk,
  &b4, &b5, &b6,     &b0, &bBackspace,
  &b7, &b8, &b9, nullptr,    &bCancel
};

const ButtonGrid_t numpadMenuGrid PROGMEM = {
  LAYOUT_BUTTON_X,
  LAYOUT_BUTTON_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_PITCH_Y,
  3,//rows
  5,//cols
  numpadMenuButtons
};

void updateCurrentEnteredFrequency(void)
{
  formatFreq(numpadMenuFrequency,b,sizeof(b),0);
  strncat_P(b,(const char*)F(" Hz"),sizeof(b)-strlen(b));
  displayText(b,LAYOUT_MODE_TEXT_X,LAYOUT_MODE_TEXT_Y,LAYOUT_MODE_TEXT_WIDTH,LAYOUT_MODE_TEXT_HEIGHT,COLOR_TEXT,COLOR_BACKGROUND,COLOR_BACKGROUND,TextJustification_e::Right);
}

void osNumpad(uint8_t new_digit)
{
  numpadMenuFrequency *= 10;
  numpadMenuFrequency += new_digit;
  updateCurrentEnteredFrequency();
}

void osBackspace(void)
{
  numpadMenuFrequency /= 10;
  updateCurrentEnteredFrequency();
}

#define NUMBER_ONSELECT_GENERATE(number) \
          void os##number(void){\
            osNumpad(number);\
          }

NUMBER_ONSELECT_GENERATE(0);
NUMBER_ONSELECT_GENERATE(1);
NUMBER_ONSELECT_GENERATE(2);
NUMBER_ONSELECT_GENERATE(3);
NUMBER_ONSELECT_GENERATE(4);
NUMBER_ONSELECT_GENERATE(5);
NUMBER_ONSELECT_GENERATE(6);
NUMBER_ONSELECT_GENERATE(7);
NUMBER_ONSELECT_GENERATE(8);
NUMBER_ONSELECT_GENERATE(9);

void osOk(void)
{
  SetActiveVfoFreq(numpadMenuFrequency);
  SaveSettingsToEeprom();
  setFrequency(numpadMenuFrequency);
  numpadMenuFrequency = NUMPAD_MENU_EXIT_FREQ;
}

void osCancel(void)
{
  numpadMenuFrequency = NUMPAD_MENU_EXIT_FREQ;
}
