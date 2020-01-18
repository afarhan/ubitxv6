#include "settings.h"

/* The ubitx is powered by an arduino nano. The pin assignment is as folows 
 *  
 */

#define ENC_A (A0)          // Tuning encoder interface
#define ENC_B (A1)          // Tuning encoder interface
#define FBUTTON (A2)        // Tuning encoder interface
#define PTT   (A3)          // Sense it for ssb and as a straight key for cw operation
#define ANALOG_KEYER (A6)   // This is used as keyer. The analog port has 4.7K pull up resistor. Details are in the circuit description on www.hfsignals.com
#define ANALOG_SPARE (A7)   // Not used yet

#define TX_RX (7)           // Pin from the Nano to the radio to switch to TX (HIGH) and RX(LOW)
#define CW_TONE (6)         // Generates a square wave sidetone while sending the CW. 
#define TX_LPF_A (5)        // The 30 MHz LPF is permanently connected in the output of the PA... 
#define TX_LPF_B (4)        //  ...Alternatively, either 3.5 MHz, 7 MHz or 14 Mhz LPFs are...
#define TX_LPF_C (3)        //  ...switched inline depending upon the TX frequency
#define CW_KEY (2)          //  Pin goes high during CW keydown to transmit the carrier. 
                            // ... The CW_KEY is needed in addition to the TX/RX key as the...
                            // ...key can be up within a tx period


/** pin assignments
14  T_IRQ           2 std   changed
13  T_DOUT              (parallel to SOD/MOSI, pin 9 of display)
12  T_DIN               (parallel to SDI/MISO, pin 6 of display)
11  T_CS            9   (we need to specify this)
10  T_CLK               (parallel to SCK, pin 7 of display)
9   SDO(MSIO) 12    12  (spi)
8   LED       A0    8   (not needed, permanently on +3.3v) (resistor from 5v, 
7   SCK       13    13  (spi)
6   SDI       11    11  (spi)
5   D/C       A3    7   (changable)
4   RESET     A4    9 (not needed, permanently +5v)
3   CS        A5    10  (changable)
2   GND       GND
1   VCC       VCC

The model is called tjctm24028-spi
it uses an ILI9341 display controller and an  XPT2046 touch controller.
*/

#define TFT_DC  9
#define TFT_CS 10
#define CS_PIN  8     //this is the pin to select the touch controller on spi interface

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
extern char c[30], b[30];

/** 
 *  The second set of 16 pins on the Raduino's bottom connector are have the three clock outputs and the digital lines to control the rig.
 *  This assignment is as follows :
 *    Pin   1   2    3    4    5    6    7    8    9    10   11   12   13   14   15   16
 *         GND +5V CLK0  GND  GND  CLK1 GND  GND  CLK2  GND  D2   D3   D4   D5   D6   D7  
 *  These too are flexible with what you may do with them, for the Raduino, we use them to :
 *  - TX_RX line : Switches between Transmit and Receive after sensing the PTT or the morse keyer
 *  - CW_KEY line : turns on the carrier for CW
 */


/**
 * The uBITX is an upconnversion transceiver. The first IF is at 45 MHz.
 * The first IF frequency is not exactly at 45 Mhz but about 5 khz lower,
 * this shift is due to the loading on the 45 Mhz crystal filter by the matching
 * L-network used on it's either sides.
 * The first oscillator works between 48 Mhz and 75 MHz. The signal is subtracted
 * from the first oscillator to arriive at 45 Mhz IF. Thus, it is inverted : LSB becomes USB
 * and USB becomes LSB.
 * The second IF of 11.059 Mhz has a ladder crystal filter. If a second oscillator is used at 
 * 56 Mhz (appox), the signal is subtracted FROM the oscillator, inverting a second time, and arrives 
 * at the 11.059 Mhz ladder filter thus doouble inversion, keeps the sidebands as they originally were.
 * If the second oscillator is at 33 Mhz, the oscilaltor is subtracated from the signal, 
 * thus keeping the signal's sidebands inverted. The USB will become LSB.
 * We use this technique to switch sidebands. This is to avoid placing the lsbCarrier close to
 * 11 MHz where its fifth harmonic beats with the arduino's 16 Mhz oscillator's fourth harmonic
 */

#define INIT_USB_FREQ   (11059200l)
// limits the tuning and working range of the ubitx between 3 MHz and 30 MHz
#define LOWEST_FREQ   (100000l)
#define HIGHEST_FREQ (30000000l)
static const uint32_t THRESHOLD_USB_LSB = 10000000L;

extern unsigned long firstIF;

extern uint8_t menuOn;

/* these are functions implemented in the main file named as ubitx_xxx.ino */
void active_delay(int delay_by);
void saveVFOs();
void setFrequency(unsigned long f);
void startTx(TuningMode_e tx_mode);
void stopTx();
void ritEnable(unsigned long f);
void ritDisable();
void checkCAT();
void cwKeyer(void);
void switchVFO(Vfo_e vfoSelect);

int enc_read(void); // returns the number of ticks in a short interval, +ve in clockwise, -ve in anti-clockwise
void enc_setup(void);  // Setups up initial values and interrupts.
int btnDown(); //returns true if the encoder button is pressed

/* these functions are called universally to update the display */
void updateDisplay(); //updates just the VFO frequency to show what is in 'frequency' variable
void redrawVFOs();    //redraws only the changed digits of the vfo
void guiUpdate();     //repaints the entire screen. Slow!!
void drawCommandbar(char *text);
void drawTx();
//getValueByKnob() provides a reusable dialog box to get a value from the encoder, the prefix and postfix 
//are useful to concatanate the values with text like "Set Freq to " x " KHz"
int getValueByKnob(int minimum, int maximum, int step_size,  int initial, char* prefix, char *postfix);

//functions of the setup menu. implemented in seteup.cpp
void doSetup2(); //main setup function, displays the setup menu, calls various dialog boxes
void setupBFO();
void setupFreq();

//main functions to check if any button is pressed and other user interface events
void doCommands();  //does the commands with encoder to jump from button to button
void  checkTouch(); //does the commands with a touch on the buttons



/* these are functiosn implemented in ubitx_si5351.cpp */
void si5351bx_setfreq(uint8_t clknum, uint32_t fout);
void initOscillators();
void si5351_set_calibration(int32_t cal); //calibration is a small value that is nudged to make up for the inaccuracies of the reference 25 MHz crystal frequency 
