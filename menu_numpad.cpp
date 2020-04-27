#include "menu_numpad.h"
#include "menu_numpad_buttons.h"

#include <avr/pgmspace.h>

#include "color_theme.h"
#include "menu_np_ql_shared.h"
#include "menu_utils.h"
#include "nano_gui.h"

void initNumpad(void);
MenuReturn_e runNumpad(const ButtonPress_e tuner_button,
                       const ButtonPress_e touch_button,
                       const Point touch_point,
                       const int16_t knob);
Menu_t numpad_menu = {
  initNumpad,
  runNumpad,
  nullptr
};

Menu_t *const numpadMenu = &numpad_menu;

int16_t numpadMenuSelectedItemRaw = 0;//Allow negative only for easier checks on wrap around

void drawNumpad(void)
{
  displayFillrect(0,47,320,200,COLOR_BACKGROUND);
  Button button;
  Button* bp;
  for(uint8_t i = 0; i < NUMPAD_MENU_NUM_BUTTONS; ++i){
    memcpy_P(&bp, &(numpadMenuButtons[i]), sizeof(bp));
    extractAndDrawButton(&button,bp);
  }
}

void initNumpad(void)
{
  numpadMenuFrequency = 0;
  numpadSelectionMode = ButtonPress_e::LongPress;//Anything except NotPressed
  drawNumpad();
  initSelector(&numpadMenuSelectedItemRaw,
                numpadMenuButtons,
                NUMPAD_MENU_NUM_BUTTONS,
                MorsePlaybackType_e::PlayChar);
}

MenuReturn_e runNumpad(const ButtonPress_e tuner_button,
                       const ButtonPress_e touch_button,
                       const Point touch_point,
                       const int16_t knob)
{
  return runNpQlShared(tuner_button,
                       touch_button,
                       touch_point,
                       knob,
                       &numpadMenuSelectedItemRaw,
                       numpadMenuButtons,
                       NUMPAD_MENU_NUM_BUTTONS,
                       &numpadSelectionMode);
}