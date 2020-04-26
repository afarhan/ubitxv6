#include "menu_utils.h"

#include <avr/pgmspace.h>

#include "button.h"
#include "color_theme.h"
#include "morse.h"
#include "nano_gui.h"
#include "utils.h"

bool findPressedButton(const Button* const* buttons,
                       const uint8_t num_buttons,
                       Button *const button_out,
                       const Point touch_point)
{
  Button* bp;
  for(uint16_t i = 0; i < num_buttons; ++i){
    memcpy_P(&bp,&(buttons[i]),sizeof(bp));
    memcpy_P(button_out,bp,sizeof(*button_out));
    if((button_out->x <= touch_point.x)
     &&(touch_point.x <= button_out->x + button_out->w)
     &&(button_out->y <= touch_point.y)
     &&(touch_point.y <= button_out->y + button_out->h)){
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

  const ButtonStatus_e bs = button->status();
  if(ButtonStatus_e::Inactive == bs){
    morseBool(false);
  }
  else if(ButtonStatus_e::Active == bs){
    morseBool(true);
  }
}

void initSelector(int16_t *const raw_select_val_in_out,
                  const Button* const* buttons,
                  const uint8_t num_buttons,
                  const MorsePlaybackType_e play_type)
{
  *raw_select_val_in_out = 0;
  if(0 < num_buttons){
    Button button;
    Button* bp;
    memcpy_P(&bp,&(buttons[0]),sizeof(bp));
    memcpy_P(&button,bp,sizeof(button));
    movePuck(nullptr,&button);
    playButtonMorse(&button,play_type);
  }
}

void adjustSelector(int16_t *const raw_select_val_in_out,
                    const int16_t knob,
                    const Button* const* buttons,
                    const uint8_t num_buttons,
                    const MorsePlaybackType_e play_type)
{
  const uint8_t prev_select = (*raw_select_val_in_out)/MENU_KNOB_COUNTS_PER_ITEM;
  *raw_select_val_in_out = LIMIT((*raw_select_val_in_out)+knob,0,num_buttons*MENU_KNOB_COUNTS_PER_ITEM - 1);
  const uint8_t new_select = (*raw_select_val_in_out)/MENU_KNOB_COUNTS_PER_ITEM;
  if(prev_select != new_select){
    Button prev_button;
    Button* bp;
    memcpy_P(&bp,&(buttons[prev_select]),sizeof(bp));
    memcpy_P(&prev_button,bp,sizeof(prev_button));
    Button new_button;
    memcpy_P(&bp,&(buttons[new_select]),sizeof(bp));
    memcpy_P(&new_button,bp,sizeof(new_button));

    movePuck(&prev_button,&new_button);
    playButtonMorse(&new_button,play_type);
  }
}

void endSelector(const Button *const button)
{
  movePuck(button,nullptr);
}
