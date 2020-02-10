#include "ui_touch.h"

#include <Arduino.h>//delay

#include "button_timing.h"
#include "touch.h"

ButtonPress_e checkTouch(Point *const touch_point_out){
  if (!readTouch(touch_point_out)){
    return ButtonPress_e::NotPressed;
  }
  delay(DEBOUNCE_DELAY_MS);
  if (!readTouch(touch_point_out)){//debounce
    return ButtonPress_e::NotPressed;
  }
  
  uint16_t down_time = 0;
  while(readTouch(touch_point_out) && (down_time < LONG_PRESS_TIME_MS)){
    delay(LONG_PRESS_POLL_TIME_MS);
    down_time += LONG_PRESS_POLL_TIME_MS;
  }

  scaleTouch(touch_point_out);

  if(down_time < LONG_PRESS_TIME_MS){
    return ButtonPress_e::ShortPress;
  }
  else{
    return ButtonPress_e::LongPress;
  }
}
