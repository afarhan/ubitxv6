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
#include <EEPROM.h>
#include "ubitx.h"

/**
    The main chip which generates upto three oscillators of various frequencies in the
    Raduino is the Si5351a. To learn more about Si5351a you can download the datasheet
    from www.silabs.com although, strictly speaking it is not a requirment to understand this code.

    We no longer use the standard SI5351 library because of its huge overhead due to many unused
    features consuming a lot of program space. Instead of depending on an external library we now use
    Jerry Gaffke's, KE7ER, lightweight standalone mimimalist "si5351bx" routines (see further down the
    code). Here are some defines and declarations used by Jerry's routines:
*/


/**
 * We need to carefully pick assignment of pin for various purposes.
 * There are two sets of completely programmable pins on the Raduino.
 * First, on the top of the board, in line with the LCD connector is an 8-pin connector
 * that is largely meant for analog inputs and front-panel control. It has a regulated 5v output,
 * ground and six pins. Each of these six pins can be individually programmed 
 * either as an analog input, a digital input or a digital output. 
 * The pins are assigned as follows (left to right, display facing you): 
 *      Pin 1 (Violet), A7, REF measure input
 *      Pin 2 (Blue),   A6, FWD measure input
 *      Pin 3 (Green), +5v
 *      Pin 4 (Yellow), Gnd
 *      Pin 5 (Orange), A3, LED Control output
 *      Pin 6 (Red),    A2, BY-PASS CONTROL output
 *      Pin 7 (Brown),  A1, SWR protection input
 *      Pin 8 (Black),  A0,
 *Note: A5, A4 are wired to the Si5351 as I2C interface 
 *       *     
 */

/* The model is called tjctm24028-spi
it uses an ILI9341 display controller and an  XPT2046 touch controller.
*/

/**
 * The Arduino, unlike C/C++ on a regular computer with gigabytes of RAM, has very little memory.
 * We have to be very careful with variables that are declared inside the functions as they are 
 * created in a memory region called the stack. The stack has just a few bytes of space on the Arduino
 * if you declare large strings inside functions, they can easily exceed the capacity of the stack
 * and mess up your programs. 
 * We circumvent this by declaring a few global buffers as  kitchen counters where we can 
 * slice and dice our strings. These strings are mostly used to control the display or handle
 * the input and output from the USB port. We must keep a count of the bytes used while reading
 * the serial port as we can easily run out of buffer space. This is done in the serial_in_count variable.
 */

char c[30], b[30];
char printBuff[2][20];  //mirrors what is showing on the two lines of the display
int count = 0;          //to generally count ticks, loops, etc


uint8_t by_pass = 0;

unsigned long sideTone=800, usbCarrier;
unsigned long frequency=7150000UL; //frequency is the current frequency on the dial
unsigned long firstIF =   45005000UL;

// if cwMode is flipped on, the rx frequency is tuned down by sidetone hz instead of being zerobeat
int cwMode = 0;

extern int32_t calibration;

boolean enableSWR = false;
boolean enablePTT = false;

/**
 * Raduino needs to keep track of current state of the transceiver. These are a few variables that do it
 */
char inTx = 0;                //it is set to 1 if in transmit mode (whatever the reason : cw, ptt or cat)
char isUSB = 0;               //upper sideband was selected, this is reset to the default for the 
                              //frequency when it crosses the frequency border of 10 MHz
unsigned char txFilter = 0;   //which of the four transmit filters are in use
boolean modeCalibrate = false;//this mode of menus shows extended menus to calibrate the oscillators and choose the proper
                              //beat frequency

int reflected;
int forward;

boolean isHighSWR = false;

/**
 * Below are the basic functions that control the uBitx. Understanding the functions before 
 * you start hacking around
 */

void saveVFOs(){

    EEPROM.put(VFO, frequency);
    if (isUSB == 1)
        EEPROM.put(VFO_MODE, VFO_MODE_USB);
    else
        EEPROM.put(VFO_MODE, VFO_MODE_LSB);
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

  if (freq >= 21000000L){  // the default filter is with 35 MHz cut-off
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0);
    txFilter = 0;
  }
  else if (freq >= 14000000L){ //thrown the KT1 relay on, the 30 MHz LPF is bypassed and the 14-18 MHz LPF is allowd to go through
    digitalWrite(TX_LPF_A, 1);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0);
    txFilter = 'A';
  }
  else if (freq >= 4500000L){
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 1);
    digitalWrite(TX_LPF_C, 0);
    txFilter = 'B';
  }
  else {
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 1);
    txFilter = 'C';
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

void setFrequency(unsigned long f){

  setTXFilters(f);

  if (isUSB){
      si5351bx_setfreq(2, firstIF  + f);
      si5351bx_setfreq(1, firstIF + usbCarrier);
  }
  else{
      si5351bx_setfreq(2, firstIF + f);
      si5351bx_setfreq(1, firstIF - usbCarrier);
  }

  frequency = f;
}

/**
 * startTx is called by the PTT, cw keyer and CAT protocol to
 * put the uBitx in tx mode. It takes care of rit settings, sideband settings
 * Note: In cw mode, doesnt key the radio, only puts it in tx mode
 * CW offest is calculated as lower than the operating frequency when in LSB mode, and vice versa in USB mode
 */

void startTx(){
    digitalWrite(TX_RX, 1);
    inTx = 1;

    setFrequency(frequency);
}

void stopTx(){
    digitalWrite(TX_RX, 0);           //turn off the tx
    inTx = 0;

    si5351bx_setfreq(0, usbCarrier);  //set back the cardrier oscillator anyway, cw tx switches it off

    setFrequency(frequency);
}


/**
 * The settings are read from EEPROM. The first time around, the values may not be 
 * present or out of range, in this case, some intelligent defaults are copied into the 
 * variables.
 */
void initSettings(){
    byte x;

    //read the settings from the eeprom and restore them
    //if the readings are off, then set defaults
    EEPROM.get(MASTER_CAL, calibration);
    // CHECK calibration value !!! TODO

    EEPROM.get(USB_CAL, usbCarrier);

    if (usbCarrier > 11060000l || usbCarrier < 11048000l)
        usbCarrier = 11052000l;

    EEPROM.get(BYPASS_STATE, x);

    switch(x){
    case 0:
        by_pass = 0;
        break;
    case 1:
        by_pass = 1;
        break;
    default:
        by_pass = 1;
    }


    // EEPROM.get(RESERVED, isHighSWR); // ??
    EEPROM.get(VFO, frequency);


    if (frequency > 40000000l || 500000l > frequency) // bigger than 40 MHz and smaller than 500 kHz
        frequency = 7150000l;

    /*
     * The VFO modes are read in as either 2 (USB) or 3(LSB), 0, the default
     * is taken as 'uninitialized
     */

    EEPROM.get(VFO_MODE, x);

    switch(x){
    case VFO_MODE_USB:
        isUSB = 1;
        break;
    case VFO_MODE_LSB:
        isUSB = 0;
        break;
    default:
        isUSB = 0;
    }

}

void initPorts(){

  analogReference(DEFAULT);

//  pinMode(PTT, INPUT_PULLUP);

  pinMode(SWR_PROT, INPUT);

  pinMode(ANALOG_FWD, INPUT);
  pinMode(ANALOG_REF, INPUT);

  pinMode(BY_PASS, OUTPUT);
  digitalWrite(BY_PASS, by_pass);

  pinMode(LED_CONTROL, OUTPUT);
  digitalWrite(LED_CONTROL, 0);

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
  initPorts();
  initOscillators();
  setFrequency(frequency);
}


void checkSWR(byte ref)
{
    if (ref == 0)
        forward = analogRead(ANALOG_FWD);
    else
        reflected = analogRead(ANALOG_REF);

#if 0
    // TODO
    // implement TX reflected protection here!!!
    if (reflected > 0)
    {
        stopTx();
        isHighSWR = true;
    }
#endif

}
/**
 * The loop checks for keydown, ptt, function button and tuning.
 */

boolean wastouched = false;
int swr_pace = 0;

void loop(){

// this should go to a task..
#if 0
    if (enableSWR)
    {
        if ((swr_pace++ % 2000) == 500)
            checkSWR(0);
        if ((swr_pace % 2000) == 1500)
            checkSWR(1);
    }
#endif
    checkCAT();
}
