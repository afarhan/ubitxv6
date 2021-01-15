 /**
 * This source file is under General Public License version 3.
 * 
 * This verision uses a built-in Si5351 library
 * Most source code are meant to be understood by the compilers and the computers. 
 * Code that has to be hackable needs to be well understood and properly documented. 
 * Donald Knuth coined the term Literate Programming to indicate code that is written be 
 * easily read and understood.
 * 
 * The Raduino is a small board that includes the Arduin Nano, a TFT display and
 * an Si5351a frequency synthesizer. This board is manufactured by HF Signals Electronics Pvt Ltd
 * 
 * To learn more about Arduino you may visit www.arduino.cc. 
 * 
 * The Arduino works by starts executing the code in a function called setup() and then it 
 * repeatedly keeps calling loop() forever. All the initialization code is kept in setup()
 * and code to continuously sense the tuning knob, the function button, transmit/receive,
 * etc is all in the loop() function. If you wish to study the code top down, then scroll
 * to the bottom of this file and read your way up.
 * 
 * Below are the libraries to be included for building the Raduino 
 * The EEPROM library is used to store settings like the frequency memory, caliberation data, etc.
 *
 *  The main chip which generates upto three oscillators of various frequencies in the
 *  Raduino is the Si5351a. To learn more about Si5351a you can download the datasheet 
 *  from www.silabs.com although, strictly speaking it is not a requirment to understand this code. 
 *  Instead, you can look up the Si5351 library written by xxx, yyy. You can download and 
 *  install it from www.url.com to complile this file.
 *  The Wire.h library is used to talk to the Si5351 and we also declare an instance of 
 *  Si5351 object to control the clocks.
 */
#include <Wire.h>
#include "encoder.h"
#include "menu.h"
#include "menu_main.h"
#include "morse.h"
#include "pin_definitions.h"
#include "push_button.h"
#include "nano_gui.h"
#include "settings.h"
#include "setup.h"
#include "si5351.h"
#include "touch.h"
#include "tuner.h"
#include "ui_touch.h"

/**
 * The Arduino, unlike C/C++ on a regular computer with gigabytes of RAM, has very little memory.
 * We have to be very careful with variables that are declared inside the functions as they are 
 * created in a memory region called the stack. The stack has just a few bytes of space on the Arduino
 * if you declare large strings inside functions, they can easily exceed the capacity of the stack
 * and mess up your programs. 
 * We circumvent this by declaring a few global buffers as kitchen counters where we can 
 * slice and dice our strings. These strings are mostly used to control the display or handle
 * the input and output from the USB port. We must keep a count of the bytes used while reading
 * the serial port as we can easily run out of buffer space. This is done in the serial_in_count variable.
 */
char b[128];
char c[30];

//during CAT commands, we will freeeze the display until CAT is disengaged
unsigned char doingCAT = 0;

/**
 * Basic User Interface Routines. These check the front panel for any activity
 */

/**
 * The PTT is checked only if we are not already in a cw transmit session
 * If the PTT is pressed, we shift to the ritbase if the rit was on
 * flip the T/R line to T and update the display to denote transmission
 */

void checkPTT(){
  //we don't check for ptt when transmitting cw
  if (globalSettings.cwExpirationTimeMs > 0){
    return;
  }

  if(digitalRead(PIN_PTT) == 0 && !globalSettings.txActive){
    startTx(TuningMode_e::TUNE_SSB);
    delay(50); //debounce the PTT
  }

  if (digitalRead(PIN_PTT) == 1 && globalSettings.txActive)
    stopTx();
}

/**
 * The settings are read from EEPROM. The first time around, the values may not be 
 * present or out of range, in this case, some intelligent defaults are copied into the 
 * variables.
 */
void initSettings(){
  LoadDefaultSettings();
  LoadSettingsFromEeprom();
}

void initPorts(){

  analogReference(DEFAULT);

  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_ENC_PUSH_BUTTON, INPUT_PULLUP);
  enc_setup();

  pinMode(PIN_PTT, INPUT_PULLUP);

  pinMode(PIN_ANALOG_REF, INPUT);
  pinMode(PIN_ANALOG_FWD, INPUT);

  pinMode(PIN_CW_TONE, OUTPUT);
  digitalWrite(PIN_CW_TONE, 0);

  pinMode(PIN_TX_RXn,OUTPUT);
  digitalWrite(PIN_TX_RXn, 0);

  pinMode(PIN_TX_LPF_A, OUTPUT);
  pinMode(PIN_TX_LPF_B, OUTPUT);
  pinMode(PIN_TX_LPF_C, OUTPUT);
  digitalWrite(PIN_TX_LPF_A, 0);
  digitalWrite(PIN_TX_LPF_B, 0);
  digitalWrite(PIN_TX_LPF_C, 0);

  pinMode(PIN_CW_KEY, OUTPUT);
  digitalWrite(PIN_CW_KEY, 0);

}

void setup()
{
  Serial.begin(38400);
  Serial.flush();

  initSettings();
  initPorts();
  displayInit();
  initTouch();
  initOscillators();
  setFrequency(globalSettings.vfoA.frequency);

  //Run initial calibration routine if button is pressed during power up
  if(ButtonPress_e::NotPressed != CheckTunerButton()){
    LoadDefaultSettings();
    setupTouch();
    SetActiveVfoMode(VfoMode_e::VFO_MODE_USB);
    setFrequency(10000000L);
    runLocalOscSetting();
    SetActiveVfoMode(VfoMode_e::VFO_MODE_LSB);
    setFrequency(7100000L);
    runBfoSetting();
  }

  rootMenu->initMenu();
}


/**
 * The loop checks for keydown, ptt, function button and tuning.
 */

void loop(){
  if(TuningMode_e::TUNE_CW == globalSettings.tuningMode){
    cwKeyer();
  }
  else if(!globalSettings.txCatActive){
    checkPTT();
  }

  checkCAT();

  if(globalSettings.txActive){
    //Don't run menus when transmitting
    return;
  }
    
  ButtonPress_e tuner_button = CheckTunerButton();
  Point touch_point;
  ButtonPress_e touch_button = checkTouch(&touch_point);
  int16_t knob = enc_read();
  runActiveMenu(tuner_button,touch_button,touch_point,knob);
}
