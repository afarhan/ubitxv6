 /**
 * This source file is under General Public License version 3.
 *
 * Modified by Rafael Diniz <rafael@rhizomatica.org>
 *
 * Original file by Farhan's original ubitxv6 firmware
 *
 * The ubitx is powered by an arduino nano. The pin assignment is as folows 
 *
 */

#define ANT_GOOD  (A0)        // Signal to display antenna green led
#define ANT_HIGH_SWR    (A1)        // Signal to display antenna red led
#define BY_PASS     (A2)        // PA by-pass
#define LED_CONTROL (A3)        // LED control light
#define ANALOG_FWD  (A6)        // Forward power measure
#define ANALOG_REF  (A7)        // Reflected power measure

/**
 *  The second set of 16 pins on the Raduino's bottom connector are have the three clock outputs and the digital lines to control the rig.
 *  This assignment is as follows :
 *    Pin   1   2    3    4    5    6    7    8    9    10   11   12   13   14   15   16
 *         GND +5V CLK0  GND  GND  CLK1 GND  GND  CLK2  GND  D2   D3   D4   D5   D6   D7  
 *  These too are flexible with what you may do with them, for the Raduino, we use them to :
 *  - TX_RX line : Switches between Transmit and Receive after sensing the PTT or the morse keyer
 *  - CW_KEY line : turns on the carrier for CW
 */


#define TX_RX    (7)           // Pin from the Nano to the radio to switch to TX (HIGH) and RX(LOW)
#define CW_TONE  (6)         // Generates a square wave sidetone while sending the CW. 
// #define TX_LPF_A (5)        // The 30 MHz LPF is permanently connected in the output of the PA... 
#define CAL_CLK (5)        // CLK #0
#define TX_LPF_B (4)        //  ...Alternatively, either 3.5 MHz, 7 MHz or 14 Mhz LPFs are...
#define TX_LPF_C (3)        //  ...switched inline depending upon the TX frequency
// #define CW_KEY   (2)          //  Pin goes high during CW keydown to transmit the carrier. 
#define PPS_IN   (2)          //  GPS 1PPS input


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
**/

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
 * These are the indices where these user changable settinngs are stored  in the EEPROM
 */
#define MASTER_CAL 0
#define LSB_CAL 4
#define USB_CAL 8
#define VFO 12
#define VFO_MODE  16 // 2: LSB, 3: USB
#define BYPASS_STATE 20 // 1- bypass
#define SERIAL_NR 24 // serial number
#define REF_THRESHOLD 28 // reflected threshold for protection

#define EEPROM_LAST 30 // EEPROM main settings size

// following are the addresses for default values store in the EEPROM
#define DEFAULT_MASTER_CAL 40
#define DEFAULT_LSB_CAL 44
#define DEFAULT_USB_CAL 48
#define DEFAULT_VFO 52
#define DEFAULT_VFO_MODE  56
#define DEFAULT_BYPASS_STATE 60
#define DEFAULT_SERIAL_NR 64
#define DEFAULT_REF_THRESHOLD 68

#define DEFAULTS_OFFSET 40

// values stored for VFO modes
#define VFO_MODE_LSB 2
#define VFO_MODE_USB 3


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

#define INIT_USB_FREQ 11052000UL
// limits the tuning and working range of the ubitx between 3 MHz and 30 MHz
#define LOWEST_FREQ     500000UL
#define HIGHEST_FREQ 109000000UL

// used to signal no reading made...
#define NO_MEASURE 65535

#define SENSORS_READ_FREQ 50 // ms

#define LED_BLINK_DUR 1000 // ms

extern uint32_t usbCarrier;
extern uint32_t frequency;  //frequency is the current frequency on the dial
extern uint32_t firstIF;
extern uint8_t by_pass;

extern int32_t calibration;

/**
 * Raduino needs to keep track of current state of the transceiver. These are a few variables that do it
 */
extern char inTx;                //it is set to 1 if in transmit mode (whatever the reason : cw, ptt or cat)
extern char isUSB;               //upper sideband was selected, this is reset to the default for the 
extern unsigned char txFilter;   //which of the four transmit filters are in use

extern uint8_t by_pass; // PA by-pass

extern boolean is_swr_protect_enabled;

extern uint16_t reflected;
extern uint16_t forward;

extern uint8_t led_status;

extern uint8_t led_antenna_red;
extern uint8_t led_antenna_green;

extern uint32_t serial;

extern uint16_t reflected_threshold;

extern uint32_t milisec_count;

extern unsigned int tcount;


/* these are functions implemented in the main file named as ubitx_xxx.ino */

void setFrequency(unsigned long f);
void startTx();
void stopTx();

// eeprom stuff
void set_radio_defaults();
void restore_radio_defaults();
void setMasterCal(int32_t calibration_offset);
void setBFO(uint32_t usbcarrier_freq);
void saveVFOs();
void save_reflected_threshold();
void read_settings_from_eeprom();

void checkTimers();
void checkSWRProtection();
void checkFWD();
void checkREF();
void setLed(boolean enabled);
void setPAbypass(boolean enabled);
void triggerProtectionReset();
void setSerial(unsigned long serial_nr);

/* these are functiosn implemented in ubitx_si5351.cpp */
void si5351bx_setfreq(uint8_t clknum, uint32_t fout);
void initOscillators();
void si5351_set_calibration(int32_t cal); //calibration is a small value that is nudged to make up for the inaccuracies of the reference 25 MHz crystal frequency 
