#include "menu_utils.h"

#include <avr/pgmspace.h>

#include "button.h"
#include "color_theme.h"
#include "morse.h"
#include "nano_gui.h"
#include "utils.h"

bool runSubmenu(Menu_t* const current_menu,
                void(*const redraw_callback)(),
                const ButtonPress_e tuner_button,
                const ButtonPress_e touch_button,
                const Point touch_point,
                const int16_t knob){
  if(nullptr != current_menu->active_submenu){
    auto ret = current_menu->active_submenu->runMenu(tuner_button,touch_button,touch_point,knob);
    switch(ret){
      case MenuReturn_e::StillActive://Fallthrough intended
      case MenuReturn_e::ExitedNoRedraw:
      {
        //Nothing to do here - just return
        break;
      }
      default://Fallthrough intended. Default to this menu being active
      case MenuReturn_e::ExitedRedraw:
      {
        //Turn off submenu, redraw, then return
        current_menu->active_submenu = nullptr;
        redraw_callback();
        break;
      }
    }//end switch
    return true;
  }//end submenu
  return false;
}

bool findPressedButton(const Button *const buttons,
                       const uint8_t num_buttons,
                       Button *const button_out,
                       const Point touch_point)
{
  for(uint16_t i = 0; i < num_buttons; ++i){
    if((buttons[i].x <= touch_point.x)
     &&(touch_point.x <= buttons[i].x + buttons[i].w)
     &&(buttons[i].y <= touch_point.y)
     &&(touch_point.y <= buttons[i].y + buttons[i].h)){
       memcpy_P(button_out,&buttons[i],sizeof(button_out));
       return true;
     }
  }

  return false;
}

void movePuck(const Button *const b_old,
              const Button *const b_new)
{
  if(nullptr != b_old){
    displayRect(b_old->x,b_old->y,b_old->w,b_old->h,COLOR_INACTIVE_BORDER);
  }
  if(nullptr != b_new){
    displayRect(b_new->x,b_new->y,b_new->w,b_new->h,COLOR_ACTIVE_BORDER);
  }
}

void playButtonMorse(const Button *const button,
                     const MorsePlaybackType_e play_type)
{
  if(MorsePlaybackType_e::PlayText == play_type){
    morseText(button->text);
  }
  else{
    morseLetter(button->morse);
  }

  int8_t morse_status = 0;
  button->morse_status(&morse_status);
  if(morse_status < 0){
    morseBool(false);
  }
  else if(morse_status > 0){
    morseBool(true);
  }
}

void initSelector(int16_t *const raw_select_val_in_out,
                  const Button *const buttons,
                  const uint8_t num_buttons,
                  const MorsePlaybackType_e play_type)
{
  *raw_select_val_in_out = 0;
  if(0 < num_buttons){
    Button button;
    memcpy_P(&button,&buttons[0],sizeof(button));
    playButtonMorse(&button,play_type);
  }
}

void adjustSelector(int16_t *const raw_select_val_in_out,
                    const int16_t knob,
                    const Button *const buttons,
                    const uint8_t num_buttons,
                    const MorsePlaybackType_e play_type)
{
  const uint8_t prev_select = (*raw_select_val_in_out)/MENU_KNOB_COUNTS_PER_ITEM;
  *raw_select_val_in_out = LIMIT((*raw_select_val_in_out)+knob,0,num_buttons*MENU_KNOB_COUNTS_PER_ITEM - 1);
  const uint8_t new_select = (*raw_select_val_in_out)/MENU_KNOB_COUNTS_PER_ITEM;
  if(prev_select != new_select){
    Button prev_button;
    memcpy_P(&prev_button,&buttons[prev_select],sizeof(prev_button));
    Button new_button;
    memcpy_P(&new_button,&buttons[new_select],sizeof(new_button));

    movePuck(&prev_button,&new_button);
    playButtonMorse(&new_button,play_type);
  }
}
