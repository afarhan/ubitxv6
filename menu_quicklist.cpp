#include "menu_quicklist.h"
#include "menu_quicklist_buttons.h"

#include <avr/pgmspace.h>
#include <WString.h>//F()

#include "color_theme.h"
#include "menu_utils.h"
#include "nano_gui.h"
#include "scratch_space.h"

void initQuickList(void);
MenuReturn_e runQuickList(const ButtonPress_e tuner_button,
                       const ButtonPress_e touch_button,
                       const Point touch_point,
                       const int16_t knob);
Menu_t quickList_menu = {
  initQuickList,
  runQuickList,
  nullptr
};

Menu_t *const quickListMenu = &quickList_menu;

int16_t quickListMenuSelectedItemRaw = 0;//Allow negative only for easier checks on wrap around

void drawQuickList(void)
{
  displayFillrect(0,47,320,200,COLOR_BACKGROUND);
  Button button;
  Button* bp;
  for(uint8_t i = 0; i < QUICKLIST_MENU_NUM_BUTTONS; ++i){
    memcpy_P(&bp, &(quickListMenuButtons[i]), sizeof(bp));
    memcpy_P(&button,bp,sizeof(button));
    drawButton(&button);
  }
  strncpy_P(b,(const char*)F("Short press = load\nLong press = save"),sizeof(b));
  displayText(b,10,47,170,200,COLOR_TEXT,COLOR_BACKGROUND,COLOR_BACKGROUND);
}

void initQuickList(void)
{
  quickListSelectionMode = ButtonPress_e::ShortPress;//Anything except NotPressed
  drawQuickList();
  initSelector(&quickListMenuSelectedItemRaw,
                quickListMenuButtons,
                QUICKLIST_MENU_NUM_BUTTONS,
                MorsePlaybackType_e::PlayChar);
}

MenuReturn_e runQuickList(const ButtonPress_e tuner_button,
                       const ButtonPress_e touch_button,
                       const Point touch_point,
                       const int16_t knob)
{
  if(ButtonPress_e::NotPressed != tuner_button){
    uint8_t menu_index = quickListMenuSelectedItemRaw/MENU_KNOB_COUNTS_PER_ITEM;
    Button button;
    Button* bp;
    memcpy_P(&bp,&(quickListMenuButtons[menu_index]),sizeof(bp));
    memcpy_P(&button,bp,sizeof(button));
    quickListSelectionMode = tuner_button;
    button.on_select();
  }//tuner_button

  else if(ButtonPress_e::NotPressed != touch_button){
    Button button;
    if(findPressedButton(quickListMenuButtons,QUICKLIST_MENU_NUM_BUTTONS,&button,touch_point)){
      quickListSelectionMode = touch_button;
      button.on_select();
    }
    else{
      //Touch detected, but not on our buttons, so ignore
    }
  }//touch_button

  else{//Neither button input type found, so handle the knob
    adjustSelector(&quickListMenuSelectedItemRaw,
                    knob,
                    quickListMenuButtons,
                    QUICKLIST_MENU_NUM_BUTTONS,
                    MorsePlaybackType_e::PlayChar);
  }

  if(ButtonPress_e::NotPressed == quickListSelectionMode){
    return MenuReturn_e::ExitedRedraw;
  }
  else{
    return MenuReturn_e::StillActive;
  }
}