#include "menu.h"

#include <avr/pgmspace.h>

#include "button.h"
#include "color_theme.h"
#include "nano_gui.h"

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
