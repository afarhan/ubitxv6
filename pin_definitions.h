#pragma once

/* The ubitx is powered by an arduino nano. The pin assignment is as folows 
 *  
 */

static const uint8_t PIN_ENC_A = A0;          // Tuning encoder interface
static const uint8_t PIN_ENC_B = A1;          // Tuning encoder interface
static const uint8_t PIN_ENC_PUSH_BUTTON = A2;        // Tuning encoder interface
static const uint8_t PIN_PTT = A3;            // Sense it for ssb and as a straight key for cw operation
static const uint8_t PIN_ANALOG_KEYER = A6;   // This is used as keyer. The analog port has 4.7K pull up resistor. Details are in the circuit description on www.hfsignals.com
static const uint8_t PIN_ANALOG_SPARE = A7;   // Not used yet

static const uint8_t PIN_TX_RXn = 7;           // Pin from the Nano to the radio to switch to TX (HIGH) and RX(LOW)
static const uint8_t PIN_CW_TONE = 6;         // Generates a square wave sidetone while sending the CW. 
static const uint8_t PIN_TX_LPF_A = 5;        // The 30 MHz LPF is permanently connected in the output of the PA... 
static const uint8_t PIN_TX_LPF_B = 4;        //  ...Alternatively, either 3.5 MHz, 7 MHz or 14 Mhz LPFs are...
static const uint8_t PIN_TX_LPF_C = 3;        //  ...switched inline depending upon the TX frequency
static const uint8_t PIN_CW_KEY = 2;          //  Pin goes high during CW keydown to transmit the carrier. 
                            // ... The PIN_CW_KEY is needed in addition to the TX/RX key as the...
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

static const uint8_t PIN_TFT_DC =  9;
static const uint8_t PIN_TFT_CS = 10;
static const uint8_t PIN_TOUCH_CS =  8;     //this is the pin to select the touch controller on spi interface

int enc_read(void); // returns the number of ticks in a short interval, +ve in clockwise, -ve in anti-clockwise
void enc_setup(void);  // Setups up initial values and interrupts.
int btnDown(); //returns true if the encoder button is pressed


