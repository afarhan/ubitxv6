#include "menu_numpad.h"
#include "menu_numpad_buttons.h"

#include <avr/pgmspace.h>

#include "color_theme.h"
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
    memcpy_P(&button,bp,sizeof(button));
    drawButton(&button);
  }
}

void initNumpad(void)
{
  numpadMenuFrequency = 0;
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
  if(ButtonPress_e::NotPressed != tuner_button){
    //We treat long and short presses the same, so no need to have a switch
    uint8_t menu_index = numpadMenuSelectedItemRaw/MENU_KNOB_COUNTS_PER_ITEM;
    Button button;
    Button* bp;
    memcpy_P(&bp,&(numpadMenuButtons[menu_index]),sizeof(bp));
    memcpy_P(&button,bp,sizeof(button));
    button.on_select();
  }//tuner_button

  else if(ButtonPress_e::NotPressed != touch_button){
    //We treat long and short presses the same, so no need to have a switch
    Button button;
    if(findPressedButton(numpadMenuButtons,NUMPAD_MENU_NUM_BUTTONS,&button,touch_point)){
      button.on_select();
    }
    else{
      //Touch detected, but not on our buttons, so ignore
    }
  }//touch_button

  else{//Neither button input type found, so handle the knob
    adjustSelector(&numpadMenuSelectedItemRaw,
                    knob,
                    numpadMenuButtons,
                    NUMPAD_MENU_NUM_BUTTONS,
                    MorsePlaybackType_e::PlayChar);
  }

  if(NUMPAD_MENU_EXIT_FREQ == numpadMenuFrequency){
    return MenuReturn_e::ExitedRedraw;
  }
  else{
    return MenuReturn_e::StillActive;
  }
}