/*
* This sketch is released under the terms of the MIT Open Source License
* (see the Operation Manual for details)
*/

#include "defines.h"
#include "texts.h"
#include "display.h"
#include "heartbeat.h"
#include "main_loop.h"
#include "radio.h"


void setup() {
  // put your setup code here, to run once:

  #ifdef USE_CAT
    Serial.begin(38400, SERIAL_8N2);
    Serial.flush();
  #endif

  disp.begin();

  radio_obj.begin();

  // radio_obj.reset_mee_eeprom();

  loop_main.make_active();

  // start after loop installed for ISR handling
  rotary_begin();
  heartbeat_begin();
  initTouch();
}

void loop() {
  // put your main code here, to run repeatedly:
  loop_master::do_loop();
}
