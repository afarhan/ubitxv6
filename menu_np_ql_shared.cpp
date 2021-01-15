#include "menu_np_ql_shared.h"

#include <avr/pgmspace.h>

#include "button.h"
#include "menu_utils.h"

MenuReturn_e runNpQlShared(const ButtonPress_e tuner_button,
                           const ButtonPress_e touch_button,
                           const Point touch_point,
                           const int16_t knob,
                           int16_t *const menuSelectedItemRaw,
                           const Button *const *const menu_buttons,
                           const uint8_t menu_num_buttons,
                           ButtonPress_e *const selection_mode)
{
  if(ButtonPress_e::NotPressed != tuner_button){
    uint8_t menu_index = *menuSelectedItemRaw/MENU_KNOB_COUNTS_PER_ITEM;
    Button button;
    Button* bp;
    memcpy_P(&bp,&(menu_buttons[menu_index]),sizeof(bp));
    memcpy_P(&button,bp,sizeof(button));
    *selection_mode = tuner_button;
    button.on_select();
  }//tuner_button

  else if(ButtonPress_e::NotPressed != touch_button){
    Button button;
    if(findPressedButton(menu_buttons,menu_num_buttons,&button,touch_point)){
      *selection_mode = touch_button;
      button.on_select();
    }
    else{
      //Touch detected, but not on our buttons, so ignore
    }
  }//touch_button

  else{//Neither button input type found, so handle the knob
    adjustSelector(menuSelectedItemRaw,
                   knob,
                   menu_buttons,
                   menu_num_buttons,
                   MorsePlaybackType_e::PlayChar);
  }

  if(ButtonPress_e::NotPressed == *selection_mode){
    return MenuReturn_e::ExitedRedraw;
  }
  else{
    return MenuReturn_e::StillActive;
  }
}
