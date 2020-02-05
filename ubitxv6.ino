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
#include "morse.h"
#include "nano_gui.h"
#include "settings.h"
#include "setup.h"
#include "ubitx.h"

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
 * Below are the basic functions that control the uBitx. Understanding the functions before 
 * you start hacking around
 */

/**
 * Our own delay. During any delay, the raduino should still be processing a few times. 
 */

void active_delay(int delay_by){
  unsigned long timeStart = millis();
  while (millis() - timeStart <= (unsigned long)delay_by) {
    checkCAT();
  }
}

void saveVFOs()
{
  SaveSettingsToEeprom();
}

/**
 * Select the properly tx harmonic filters
 * The four harmonic filters use only three relays
 * the four LPFs cover 30-21 Mhz, 18 - 14 Mhz, 7-10 MHz and 3.5 to 5 Mhz
 * Briefly, it works like this, 
 * - When KT1 is OFF, the 'off' position routes the PA output through the 30 MHz LPF
 * - When KT1 is ON, it routes the PA output to KT2. Which is why you will see that
 *   the KT1 is on for the three other cases.
 * - When the KT1 is ON and KT2 is off, the off position of KT2 routes the PA output
 *   to 18 MHz LPF (That also works for 14 Mhz) 
 * - When KT1 is On, KT2 is On, it routes the PA output to KT3
 * - KT3, when switched on selects the 7-10 Mhz filter
 * - KT3 when switched off selects the 3.5-5 Mhz filter
 * See the circuit to understand this
 */

void setTXFilters(unsigned long freq){
  
  if (freq > 21000000L){  // the default filter is with 35 MHz cut-off
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0);
  }
  else if (freq >= 14000000L){ //thrown the KT1 relay on, the 30 MHz LPF is bypassed and the 14-18 MHz LPF is allowd to go through
    digitalWrite(TX_LPF_A, 1);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0);
  }
  else if (freq > 7000000L){
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 1);
    digitalWrite(TX_LPF_C, 0);
  }
  else {
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 1);
  }
}


void setTXFilters_v5(unsigned long freq){
  
  if (freq > 21000000L){  // the default filter is with 35 MHz cut-off
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0);
  }
  else if (freq >= 14000000L){ //thrown the KT1 relay on, the 30 MHz LPF is bypassed and the 14-18 MHz LPF is allowd to go through
    digitalWrite(TX_LPF_A, 1);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0);
  }
  else if (freq > 7000000L){
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 1);
    digitalWrite(TX_LPF_C, 0);
  }
  else {
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 1);
  }
}


/**
 * This is the most frequently called function that configures the 
 * radio to a particular frequeny, sideband and sets up the transmit filters
 * 
 * The transmit filter relays are powered up only during the tx so they dont
 * draw any current during rx. 
 * 
 * The carrier oscillator of the detector/modulator is permanently fixed at
 * uppper sideband. The sideband selection is done by placing the second oscillator
 * either 12 Mhz below or above the 45 Mhz signal thereby inverting the sidebands 
 * through mixing of the second local oscillator.
 */
 
void setFrequency(const unsigned long freq,
                  const bool transmit){
  static const unsigned long FIRST_IF = 45005000UL;
 
  setTXFilters(freq);

  //Nominal values for the oscillators
  uint32_t local_osc_freq = FIRST_IF + freq;
  uint32_t ssb_osc_freq = FIRST_IF;//will be changed depending on sideband
  uint32_t bfo_osc_freq = globalSettings.usbCarrierFreq;

  if(TuningMode_e::TUNE_CW == globalSettings.tuningMode){
    if(transmit){
      //We don't do any mixing or converting when transmitting
      local_osc_freq = freq;
      ssb_osc_freq = 0;
      bfo_osc_freq = 0;
    }
    else{
      //We offset when receiving CW so that it's audible
      if(VfoMode_e::VFO_MODE_USB == GetActiveVfoMode()){
        local_osc_freq -= globalSettings.cwSideToneFreq;
        ssb_osc_freq += globalSettings.usbCarrierFreq;
      }
      else{
        local_osc_freq += globalSettings.cwSideToneFreq;
        ssb_osc_freq -= globalSettings.usbCarrierFreq;
      }
    }
  }
  else{//SSB mode
    if(VfoMode_e::VFO_MODE_USB == GetActiveVfoMode()){
      ssb_osc_freq += globalSettings.usbCarrierFreq;
    }
    else{
      ssb_osc_freq -= globalSettings.usbCarrierFreq;
    }
  }

  si5351bx_setfreq(2, local_osc_freq);
  si5351bx_setfreq(1, ssb_osc_freq);
  si5351bx_setfreq(0, bfo_osc_freq);

  SetActiveVfoFreq(freq);
}

/**
 * startTx is called by the PTT, cw keyer and CAT protocol to
 * put the uBitx in tx mode. It takes care of rit settings, sideband settings
 * Note: In cw mode, doesnt key the radio, only puts it in tx mode
 * CW offest is calculated as lower than the operating frequency when in LSB mode, and vice versa in USB mode
 */
 
void startTx(TuningMode_e tx_mode){
  globalSettings.tuningMode = tx_mode;

  if (globalSettings.ritOn){
    //save the current as the rx frequency
    uint32_t rit_tx_freq = globalSettings.ritFrequency;
    globalSettings.ritFrequency = GetActiveVfoFreq();
    setFrequency(rit_tx_freq,true);
  }
  else{
    if(globalSettings.splitOn){
      if(Vfo_e::VFO_B == globalSettings.activeVfo){
        globalSettings.activeVfo = Vfo_e::VFO_A;
      }
      else{
        globalSettings.activeVfo = Vfo_e::VFO_B;
      }
    }
    setFrequency(GetActiveVfoFreq(),true);
  }

  digitalWrite(TX_RX, 1);//turn on the tx
  globalSettings.txActive = true;
  drawTx();
}

void stopTx(){
  digitalWrite(TX_RX, 0);//turn off the tx
  globalSettings.txActive = false;

  if(globalSettings.ritOn){
    uint32_t rit_rx_freq = globalSettings.ritFrequency;
    globalSettings.ritFrequency = GetActiveVfoFreq();
    setFrequency(rit_rx_freq);
  }
  else{
    if(globalSettings.splitOn){
      if(Vfo_e::VFO_B == globalSettings.activeVfo){
        globalSettings.activeVfo = Vfo_e::VFO_A;
      }
      else{
        globalSettings.activeVfo = Vfo_e::VFO_B;
      }
    }
    setFrequency(GetActiveVfoFreq());
  }
  drawTx();
}

/**
 * ritEnable is called with a frequency parameter that determines
 * what the tx frequency will be
 */
void ritEnable(unsigned long freq){
  globalSettings.ritOn = true;
  //save the non-rit frequency back into the VFO memory
  //as RIT is a temporary shift, this is not saved to EEPROM
  globalSettings.ritFrequency = freq;
}

// this is called by the RIT menu routine
void ritDisable(){
  if(globalSettings.ritOn){
    globalSettings.ritOn = false;
    setFrequency(globalSettings.ritFrequency);
    updateDisplay();
  }
}

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
    
  if(digitalRead(PTT) == 0 && !globalSettings.txActive){
    startTx(TuningMode_e::TUNE_SSB);
    active_delay(50); //debounce the PTT
  }
	
  if (digitalRead(PTT) == 1 && globalSettings.txActive)
    stopTx();
}

//check if the encoder button was pressed
void checkButton(){
  //only if the button is pressed
  if (!btnDown())
    return;
  active_delay(50);
  if (!btnDown()) //debounce
    return;

  //disengage any CAT work
  doingCAT = 0;

 int downTime = 0;
 while(btnDown()){
    active_delay(10);
    downTime++;
    if (downTime > 300){
      if(!globalSettings.morseMenuOn){
        globalSettings.morseMenuOn = true;//set before playing
        morseLetter(2);
      }
      else{
        morseLetter(4);
        globalSettings.morseMenuOn = false;//unset after playing
      }
      SaveSettingsToEeprom();
      return;
    }
  }
  active_delay(100);

  doCommands();
  //wait for the button to go up again
  while(btnDown())
    active_delay(10);
  active_delay(50);//debounce
}

void switchVFO(Vfo_e new_vfo){
  ritDisable();//If we are in RIT mode, we need to disable it before setting the active VFO so that the correct VFO gets it's frequency restored

  globalSettings.activeVfo = new_vfo;
  setFrequency(GetActiveVfoFreq());
  redrawVFOs();
  saveVFOs();
}

/**
 * The tuning jumps by 50 Hz on each step when you tune slowly
 * As you spin the encoder faster, the jump size also increases 
 * This way, you can quickly move to another band by just spinning the 
 * tuning knob
 */

void doTuning(){
  static unsigned long prev_freq;
  static unsigned long nextFrequencyUpdate = 0;

  unsigned long now = millis();
  
  if (now >= nextFrequencyUpdate && prev_freq != GetActiveVfoFreq()){
    updateDisplay();
    nextFrequencyUpdate = now + 100;
    prev_freq = GetActiveVfoFreq();
  }

  int s = enc_read();
  if (!s)
    return;

  //Serial.println(s);

  doingCAT = 0; // go back to manual mode if you were doing CAT
  prev_freq = GetActiveVfoFreq();
  uint32_t new_freq = prev_freq;

  if (s > 10 || s < -10){
    new_freq += 200L * s;
  }
  else if (s > 5 || s < -5){
    new_freq += 100L * s;
  }
  else{
    new_freq += 50L * s;
  }
  
  //Transition from below to above the traditional threshold for USB
  if(prev_freq < THRESHOLD_USB_LSB && new_freq >= THRESHOLD_USB_LSB){
    SetActiveVfoMode(VfoMode_e::VFO_MODE_USB);
  }
  
  //Transition from aboveo to below the traditional threshold for USB
  if(prev_freq >= THRESHOLD_USB_LSB && new_freq < THRESHOLD_USB_LSB){
    SetActiveVfoMode(VfoMode_e::VFO_MODE_LSB);
  }

  setFrequency(new_freq);
}


/**
 * RIT only steps back and forth by 100 hz at a time
 */
void doRIT(){
  int knob = enc_read();
  uint32_t old_freq = GetActiveVfoFreq();
  uint32_t new_freq = old_freq;

  if (knob < 0)
    new_freq -= 100l;
  else if (knob > 0)
    new_freq += 100;
 
  if (old_freq != new_freq){
    setFrequency(new_freq);
    updateDisplay();
  }
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

  //??
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(FBUTTON, INPUT_PULLUP);
  enc_setup();
  
  //configure the function button to use the external pull-up
//  pinMode(FBUTTON, INPUT);
//  digitalWrite(FBUTTON, HIGH);

  pinMode(PTT, INPUT_PULLUP);
//  pinMode(ANALOG_KEYER, INPUT_PULLUP);

  pinMode(CW_TONE, OUTPUT);  
  digitalWrite(CW_TONE, 0);
  
  pinMode(TX_RX,OUTPUT);
  digitalWrite(TX_RX, 0);

  pinMode(TX_LPF_A, OUTPUT);
  pinMode(TX_LPF_B, OUTPUT);
  pinMode(TX_LPF_C, OUTPUT);
  digitalWrite(TX_LPF_A, 0);
  digitalWrite(TX_LPF_B, 0);
  digitalWrite(TX_LPF_C, 0);

  pinMode(CW_KEY, OUTPUT);
  digitalWrite(CW_KEY, 0);
}

void setup()
{
  Serial.begin(38400);
  Serial.flush();

  initSettings();
  displayInit();
  initPorts();
  initOscillators();
  setFrequency(globalSettings.vfoA.frequency);

  //Run initial calibration routine if button is pressed during power up
  if(btnDown()){
    LoadDefaultSettings();
    setupTouch();
    SetActiveVfoMode(VfoMode_e::VFO_MODE_USB);
    setFrequency(10000000L);
    runLocalOscSetting();
    SetActiveVfoMode(VfoMode_e::VFO_MODE_LSB);
    setFrequency(7100000L);
    runBfoSetting();
  }

  guiUpdate();
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
    
  checkButton();
  //tune only when not tranmsitting 
  if(!globalSettings.txActive){
    if(globalSettings.ritOn){
      doRIT();
    }
    else{
      doTuning();
    }
    checkTouch();
  }

  checkCAT();
}
