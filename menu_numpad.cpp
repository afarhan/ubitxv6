#include "menu_numpad.h"
#include "menu_numpad_buttons.h"

#include <avr/pgmspace.h>

#include "color_theme.h"
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

void drawNumpad(void)
{
  displayFillrect(0,47,320,200,COLOR_BACKGROUND);
  Button button;
  Button* bp;
  for(uint8_t i = 0; i < NUMPAD_MENU_NUM_BUTTONS; ++i){
    memcpy_P(&bp, &(numpadMenuButtons[i]), sizeof(bp));
    memcpy_P(&button,bp,sizeof(button));
    drawButton(&button);
  }
}

void initNumpad(void)
{
  numpadMenuFrequency = 0;
  drawNumpad();
}

MenuReturn_e runNumpad(const ButtonPress_e tuner_button,
                       const ButtonPress_e touch_button,
                       const Point touch_point,
                       const int16_t knob)
{
  (void)tuner_button;(void)touch_button;(void)touch_point;(void)knob;//TODO

  if(NUMPAD_MENU_EXIT_FREQ == numpadMenuFrequency){
    return MenuReturn_e::ExitedRedraw;
  }
  else{
    return MenuReturn_e::StillActive;
  }
}