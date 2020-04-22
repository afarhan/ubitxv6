#include <Arduino.h>
#include "pin_definitions.h"

void updateDisplay() {}
void redrawVFOs(){}
void drawTx(){}
int btnDown()
{
  //Button has a pullup, so it reads high normally,
  //and reads low when pressed down
  return !digitalRead(PIN_ENC_PUSH_BUTTON);
}