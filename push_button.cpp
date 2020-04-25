#include <Arduino.h>
#include "pin_definitions.h"
#include "push_button.h"

bool IsButtonPressed()
{
  //Button has a pullup, so it reads high normally,
  //and reads low when pressed down
  return !digitalRead(PIN_ENC_PUSH_BUTTON);
}
