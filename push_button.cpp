#include <Arduino.h>
#include "button_timing.h"
#include "pin_definitions.h"
#include "push_button.h"

bool IsButtonPressed()
{
  //Button has a pullup, so it reads high normally,
  //and reads low when pressed down
  return !digitalRead(PIN_ENC_PUSH_BUTTON);
}

ButtonPress_e CheckTunerButton(){
  if (!IsButtonPressed()){
    return ButtonPress_e::NotPressed;
  }
  delay(DEBOUNCE_DELAY_MS);
  if (!IsButtonPressed()){//debounce
    return ButtonPress_e::NotPressed;
  }

  uint16_t down_time = 0;
  while(IsButtonPressed() && (down_time < LONG_PRESS_TIME_MS)){
    delay(LONG_PRESS_POLL_TIME_MS);
    down_time += LONG_PRESS_POLL_TIME_MS;
  }

  if(down_time < LONG_PRESS_TIME_MS){
    return ButtonPress_e::ShortPress;
  }
  else{
    return ButtonPress_e::LongPress;
  }
}
