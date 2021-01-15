#pragma once

/* 
 * Pin configurations for uBiTXv6
 * 
 * Attached devices are:
 *  * A rotory encoder for tuning, which has a built-in push button for selection
 *  * A Push-to-talk button input
 *  * A morse keyer input, using analog voltage divider for paddle support
 *  * A SI5351A 3-output frequency generator
 *  * A model tjctm24028-spi touch screen LCD, which has:
 *     * An ILI9341 display controller
 *     * A XPT2046 touch controller
 *  * A TX/RX output control pin
 *  * A tone output pin to provide audio feedback to the operator when sending CW
 *  * Three filter selection output control pins
 *  * A CW keydown output
 *  * Optionally, a serial CAT interface
 */

static const uint8_t PIN_ENC_A = A0;          // Tuning encoder interface
static const uint8_t PIN_ENC_B = A1;          // Tuning encoder interface
static const uint8_t PIN_ENC_PUSH_BUTTON = A2;        // Tuning encoder interface
static const uint8_t PIN_PTT = A3;            // Sense it for ssb and as a straight key for cw operation
//A4 is I2C SDA
//A5 is I2C SCK
static const uint8_t PIN_ANALOG_KEYER = A6;   // This is used as keyer. The analog port has 4.7K pull up resistor. Details are in the circuit description on www.hfsignals.com
static const uint8_t PIN_ANALOG_SPARE = A7;   // Not used yet

//13 is SPI CLK
//12 is SPI MISO
//11 is SPI MOSI
static const uint8_t PIN_TFT_CS = 10;         // Selects the LCD controller on SPI interface (active low)
static const uint8_t PIN_TFT_DC =  9;         // Tells the LCD controller if it's getting data (D, high) or commands (C, low)
static const uint8_t PIN_TOUCH_CS =  8;       // Selects the touch controller on SPI interface (active low)
static const uint8_t PIN_TX_RXn = 7;          // Pin from the Nano to the radio to switch to TX (HIGH) and RX(LOW)
static const uint8_t PIN_CW_TONE = 6;         // Generates a square wave sidetone while sending the CW
static const uint8_t PIN_TX_LPF_A = 5;        // The 30 MHz LPF is permanently connected in the output of the PA...
static const uint8_t PIN_TX_LPF_B = 4;        //  ...Alternatively, either 3.5 MHz, 7 MHz or 14 Mhz LPFs are...
static const uint8_t PIN_TX_LPF_C = 3;        //  ...switched inline depending upon the TX frequency
static const uint8_t PIN_CW_KEY = 2;          //  Pin goes high during CW keydown to transmit the carrier.
                                              // ... The PIN_CW_KEY is needed in addition to the TX/RX key as the...
                                              // ...key can be up within a tx period
//1 is UART RX
//0 is UART TX
