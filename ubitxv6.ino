 /**
 * This source file is under General Public License version 3.
 *
 * Modified by Rafael Diniz <rafael@rhizomatica.org>
 *
 * Original file by Farhan's original ubitxv6 firmware
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

/**
    The main chip which generates upto three oscillators of various frequencies in the
    Raduino is the Si5351a. To learn more about Si5351a you can download the datasheet
    from www.silabs.com although, strictly speaking it is not a requirment to understand this code.

    We no longer use the standard SI5351 library because of its huge overhead due to many unused
    features consuming a lot of program space. Instead of depending on an external library we now use
    Jerry Gaffke's, KE7ER, lightweight standalone mimimalist "si5351bx" routines (see further down the
    code). Here are some defines and declarations used by Jerry's routines:
*/

// MODIFIED BY RHIZOMATICA

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
 *      Pin 8 (Black),  A0, UNUSED

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

#include <Wire.h>
#include <EEPROM.h>
#include "ubitx.h"
#include "ubitx_cat.h"

char c[30], b[30];

uint32_t usbCarrier; // bfo
uint32_t frequency = 7150000UL; //frequency is the current frequency on the dial
uint32_t firstIF   = 45005000UL;

extern int32_t calibration; // main calibration offset


/**
 * Raduino needs to keep track of current state of the transceiver. These are a few variables that do it
 */
char inTx = 0;                //it is set to 1 if in transmit mode
char isUSB = 0;               //upper sideband was selected
uint8_t txFilter = 0;   //which of the four transmit filters are in use

uint8_t by_pass = 0; // PA by-pass

boolean is_swr_protect_enabled = false;

uint16_t reflected;
uint16_t forward;

uint8_t led_status;


/**
 * Below are the basic functions that control the uBitx. Understanding the functions before 
 * you start hacking around
 */

void setMasterCal(int32_t calibration_offset)
{
    calibration = calibration_offset;

    si5351_set_calibration(calibration);
    setFrequency(frequency);

    EEPROM.put(MASTER_CAL, calibration);

}


void setBFO(uint32_t usbcarrier_freq)
{
    usbCarrier = usbcarrier_freq;

    si5351bx_setfreq(0, usbCarrier);
    setFrequency(frequency);

    EEPROM.put(USB_CAL, usbCarrier);
}

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

  if (freq >= 21000000UL){  // the default filter is with 35 MHz cut-off
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0);
    txFilter = 0;
  }
  else if (freq >= 14000000UL){ //thrown the KT1 relay on, the 30 MHz LPF is bypassed and the 14-18 MHz LPF is allowd to go through
    digitalWrite(TX_LPF_A, 1);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0);
    txFilter = 'A';
  }
  else if (freq >= 4500000UL){
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
        by_pass = 0;
    }


    // EEPROM.get(RESERVED, isHighSWR); // ??
    EEPROM.get(VFO, frequency);


    if (frequency > 40000000UL || 500000UL > frequency) // bigger than 40 MHz and smaller than 500 kHz
        frequency = 7150000UL;

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

    // we start with the led off
    led_status = 0;

    is_swr_protect_enabled = false;
}

void initPorts(){

  analogReference(DEFAULT);

//  pinMode(PTT, INPUT_PULLUP);

  pinMode(SWR_PROT, INPUT);

  pinMode(ANALOG_FWD, INPUT);
  pinMode(ANALOG_REF, INPUT);

  pinMode(BY_PASS, OUTPUT);
  digitalWrite(BY_PASS, by_pass ? HIGH : LOW);

  pinMode(LED_CONTROL, OUTPUT);
  digitalWrite(LED_CONTROL, led_status ? HIGH : LOW);

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


void checkREF()
{
    if (inTx)
        reflected = analogRead(ANALOG_REF);
    else
        reflected = NO_MEASURE;
}

void checkFWD()
{
    if (inTx)
        forward = analogRead(ANALOG_FWD);
    else
        forward = NO_MEASURE;
}

void checkSWRProtection()
{
    uint16_t reading;
    reading = analogRead(SWR_PROT);
    // digitalRead(inPin); ?
    // The analog input pins can be used as digital pins, referred
    // to as A0, A1, etc. The exception is the Arduino Nano, Pro Mini,
    // and Mini’s A6 and A7 pins, which can only be used as analog inputs.

    // adjust this...
    if (reading > 100)
        is_swr_protect_enabled = true;
    else
        is_swr_protect_enabled = false;
}

void setLed(boolean enabled)
{
    led_status = enabled;
    digitalWrite(LED_CONTROL, led_status ? HIGH : LOW);
}
void setPAbypass(boolean enabled)
{
    by_pass = enabled;
    digitalWrite(BY_PASS, by_pass ? HIGH : LOW);
    EEPROM.put(BYPASS_STATE, by_pass);
}


/**
 * The loop checks for keydown, ptt, function button and tuning.
 */

uint16_t pace;

void loop(){

    checkCAT();

    if ((pace++ % 2000) == 500)
        checkREF();
    if ((pace % 2000) == 1500)
        checkFWD();

    if ((pace % 2000) == 1000)
        checkSWRProtection();

    // TODO 2: Block tx if swr_protection is on!

    // TODO: implement some IMALIVE feature, using the timeout from pc (if
    // no command of IMALIVE comes during a perior, so that the arduino
    // knows when the pc is dead and turns off the led

}
