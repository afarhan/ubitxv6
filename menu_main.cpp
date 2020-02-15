#include "menu_main.h"
#include "menu_main_buttons.h"

#include <avr/pgmspace.h>
#include <Arduino.h>

#include "button.h"
#include "color_theme.h"
#include "menu_utils.h"
#include "morse.h"
#include "nano_gui.h"
#include "settings.h"
#include "ubitx.h"//THRESHOLD_USB_LSB
#include "utils.h"

void drawMainMenu(void);
MenuReturn_e runMainMenu(const ButtonPress_e tuner_button,
                         const ButtonPress_e touch_button,
                         const Point touch_point,
                         const int16_t knob);
Menu_t mainMenu = {
  drawMainMenu,
  runMainMenu,
  nullptr
};

Menu_t* const rootMenu = &mainMenu;

bool mainMenuSelecting = false;//Tracks if we're selecting buttons with knob, or adjusting frequency
int16_t mainMenuSelectedItemRaw = 0;//Allow negative only for easier checks on wrap around

void drawMainMenu(void)
{
  displayClear(COLOR_BACKGROUND);
  Button button;
  Button* bp;
  for(uint8_t i = 0; i < MAIN_MENU_NUM_BUTTONS; ++i){
    memcpy_P(&bp, &(mainMenuButtons[i]), sizeof(bp));
    memcpy_P(&button,bp,sizeof(button));
    drawButton(&button);
  }
}

void mainMenuTune(int16_t knob)
{
  static uint32_t current_freq = 0;

  if((0 == knob) && (GetActiveVfoFreq() == current_freq)){
    //Nothing to do - we're already set!
    return;
  }

  current_freq = GetActiveVfoFreq();
  const uint32_t new_freq = current_freq + (50 * knob);
  
  //Transition from below to above the traditional threshold for USB
  if(current_freq < THRESHOLD_USB_LSB && new_freq >= THRESHOLD_USB_LSB){
    SetActiveVfoMode(VfoMode_e::VFO_MODE_USB);
  }
  
  //Transition from above to below the traditional threshold for USB
  if(current_freq >= THRESHOLD_USB_LSB && new_freq < THRESHOLD_USB_LSB){
    SetActiveVfoMode(VfoMode_e::VFO_MODE_LSB);
  }

  setFrequency(new_freq);
  const uint32_t old_freq = current_freq;
  current_freq = new_freq;

  Button button;
  if(Vfo_e::VFO_A == globalSettings.activeVfo){
    memcpy_P(&button,&bVfoA,sizeof(button));
  }
  else{
    memcpy_P(&button,&bVfoB,sizeof(button));
  }
  drawButton(&button);
  updateBandButtons(old_freq);
}

MenuReturn_e runMainMenu(const ButtonPress_e tuner_button,
                         const ButtonPress_e touch_button,
                         const Point touch_point,
                         const int16_t knob)
{
  if(runSubmenu(&mainMenu,
                drawMainMenu,
                tuner_button,
                touch_button,
                touch_point,
                knob)){
    //Submenu processed the input, so return now
    mainMenuSelectedItemRaw = 0;
    mainMenuSelecting = false;
    return MenuReturn_e::StillActive;//main menu always returns StillActive
  }//end submenu

  //Submenu didn't run, so handle the inputs ourselves

  //Check tuner_button
  if(ButtonPress_e::NotPressed != tuner_button){
    switch(tuner_button){
      default://Fallthrough intended
      case ButtonPress_e::NotPressed:
      {
        //Nothing to do
        break;
      }
      case ButtonPress_e::ShortPress:
      {
        if(mainMenuSelecting){
          uint8_t menu_index = mainMenuSelectedItemRaw/MENU_KNOB_COUNTS_PER_ITEM;
          Button button;
          Button* bp;
          memcpy_P(&bp,&(mainMenuButtons[menu_index]),sizeof(bp));
          memcpy_P(&button,bp,sizeof(button));
          endSelector(&button);
          button.on_select();
        }
        else{
          initSelector(&mainMenuSelectedItemRaw,
                       mainMenuButtons,
                       MAIN_MENU_NUM_BUTTONS,
                       MorsePlaybackType_e::PlayChar);
        }
        mainMenuSelecting = !mainMenuSelecting;

        //Don't handle touch or knob on this run
        return MenuReturn_e::StillActive;//main menu always returns StillActive
        break;
      }
      case ButtonPress_e::LongPress:
      {
        if(!globalSettings.morseMenuOn){
            globalSettings.morseMenuOn = true;//set before playing
            morseLetter(2);
        }
        else{
            morseLetter(4);
            globalSettings.morseMenuOn = false;//unset after playing
        }
        SaveSettingsToEeprom();
        //Don't handle touch or knob on this run
        return MenuReturn_e::StillActive;//main menu always returns StillActive
        break;
      }
    }//switch
  }//tuner_button

  else if(ButtonPress_e::NotPressed != touch_button){
    //We treat long and short presses the same, so no need to have a switch
    Button button;
    if(findPressedButton(mainMenuButtons,MAIN_MENU_NUM_BUTTONS,&button,touch_point)){
      button.on_select();
    }
    else{
      //Touch detected, but not on our buttons, so ignore
    }
  }//touch_button

  else{//Neither button input type found, so handle the knob
    if(mainMenuSelecting){
      adjustSelector(&mainMenuSelectedItemRaw,
                     knob,
                     mainMenuButtons,
                     MAIN_MENU_NUM_BUTTONS,
                     MorsePlaybackType_e::PlayChar);
    }
    else{
      mainMenuTune(knob);
    }
  }

  //

  return MenuReturn_e::StillActive;//main menu always returns StillActive
}