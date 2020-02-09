#include "menu.h"

bool runSubmenu(Menu_t* current_menu,
                void(*redraw_callback)(),
                ButtonPress_e tuner_button,
                ButtonPress_e touch_button,
                Point touch_point,
                int16_t knob){
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