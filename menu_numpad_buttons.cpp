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

uint32_t numpadMenuFrequency;
ButtonPress_e numpadSelectionMode;

#define D_STRINGIFY(x) #x
#define D_STRING(x) D_STRINGIFY(x)
#define NUMBER_BUTTON_GENERATE(number,x,y) \
          constexpr char txt##number [] PROGMEM = "" D_STRING(number);\
          ButtonStatus_e bs##number();\
          void os##number();\
          constexpr Button b##number PROGMEM = {\
            LAYOUT_BUTTON_X + x*LAYOUT_BUTTON_PITCH_X,\
            LAYOUT_BUTTON_Y + y*LAYOUT_BUTTON_PITCH_Y,\
            LAYOUT_BUTTON_WIDTH,\
            LAYOUT_BUTTON_HEIGHT,\
            txt##number,\
            nullptr,\
            bsNumpad,\
            os##number,\
            '0'+number\
          }

ButtonStatus_e bsNumpad(void){
  return ButtonStatus_e::Stateless;
}


// 1 2 3   Ok
// 4 5 6 0 <-
// 7 8 9   Can
NUMBER_BUTTON_GENERATE(1,0,0);
NUMBER_BUTTON_GENERATE(2,1,0);
NUMBER_BUTTON_GENERATE(3,2,0);
NUMBER_BUTTON_GENERATE(4,0,1);
NUMBER_BUTTON_GENERATE(5,1,1);
NUMBER_BUTTON_GENERATE(6,2,1);
NUMBER_BUTTON_GENERATE(7,0,2);
NUMBER_BUTTON_GENERATE(8,1,2);
NUMBER_BUTTON_GENERATE(9,2,2);
NUMBER_BUTTON_GENERATE(0,3,1);

constexpr char txtOk [] PROGMEM = "OK";
void osOk();
constexpr Button bOk PROGMEM = {
  LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txtOk,
  nullptr,
  bsNumpad,
  osOk,
  'K'
};

constexpr char txtBackspace [] PROGMEM = "<-";
void osBackspace();
constexpr Button bBackspace PROGMEM = {
  LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txtBackspace,
  nullptr,
  bsNumpad,
  osBackspace,
  'B'
};

constexpr char txtCancel [] PROGMEM = "Can";
void osCancel();
constexpr Button bCancel PROGMEM = {
  LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X,
  LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y,
  LAYOUT_BUTTON_WIDTH,
  LAYOUT_BUTTON_HEIGHT,
  txtCancel,
  nullptr,
  bsNumpad,
  osCancel,
  'C'
};

//Declare in menu select order, not graphical order
const Button* const numpadMenuButtons [] PROGMEM = {
  &b1, &b2, &b3, &b4, &b5, &b6, &b7, &b8, &b9, &b0,
  &bOk, &bBackspace, &bCancel
};
const uint8_t NUMPAD_MENU_NUM_BUTTONS = sizeof(numpadMenuButtons)/sizeof(numpadMenuButtons[0]);

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
  numpadSelectionMode = ButtonPress_e::NotPressed;
}

void osCancel(void)
{
  numpadSelectionMode = ButtonPress_e::NotPressed;
}
