#include "menu.h"
#include "menu_main.h"

void runActiveMenu(const ButtonPress_e tuner_button,
                   const ButtonPress_e touch_button,
                   const Point touch_point,
                   const int16_t knob)
{
  Menu_t* parent_menu = rootMenu;//rootMenu is it's own parent
  Menu_t* active_menu = rootMenu;
  while(nullptr != active_menu->active_submenu){
    parent_menu = active_menu;
    active_menu = parent_menu->active_submenu;
  }
  MenuReturn_e mr = active_menu->runMenu(tuner_button,touch_button,touch_point,knob);
  switch(mr){
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
      parent_menu->active_submenu = nullptr;
      parent_menu->initMenu();
      break;
    }
  }//end switch
}

void enterSubmenu(Menu_t *const submenu)
{
  Menu_t* current_menu = rootMenu;
  while(nullptr != current_menu->active_submenu){
    current_menu = current_menu->active_submenu;
  }
  current_menu->active_submenu = submenu;
  submenu->initMenu();
}

