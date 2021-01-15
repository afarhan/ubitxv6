#include "menu_quicklist.h"
#include "menu_quicklist_buttons.h"

#include <avr/pgmspace.h>
#include <WString.h>//F()

#include "color_theme.h"
#include "menu_np_ql_shared.h"
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
    extractAndDrawButton(&button,bp);
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
  return runNpQlShared(tuner_button,
                       touch_button,
                       touch_point,
                       knob,
                       &quickListMenuSelectedItemRaw,
                       quickListMenuButtons,
                       QUICKLIST_MENU_NUM_BUTTONS,
                       &quickListSelectionMode);
}