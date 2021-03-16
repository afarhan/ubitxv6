#include "radio.h"

#include <EEPROM.h>
#include "loop_master.h"


// **************************************
// ***** stock (v6) firmware EEPROM *****
// **************************************

// EEPROM addresses
const byte MASTER_CAL   =   0;  // long
const byte USB_CAL      =   8;  // long 
const byte VFO_A        =  16;  // long
const byte VFO_B        =  20;  // long
const byte CW_SIDETONE  =  24;  // long
const byte CW_SPEED     =  28;  // int
const byte SLOPE_X      =  32;  // int
const byte SLOPE_Y      =  36;  // int
const byte OFFSET_X     =  40;  // int
const byte OFFSET_Y     =  44;  // int
const byte CW_DELAYTIME =  48;  // int
const int VFO_A_MODE    = 256;  // byte
const int VFO_B_MODE    = 257;  // byte
const int CW_KEY_TYPE   = 358;  // byte

// **************************
// ***** MEE additional *****
// **************************

// EEPROM addresses
const int MEE_VFOA  = 359;
const int MEE_VFOB  = 360;
const int MEE_DATA1 = 361;

// band memories  364 .. 453
// 5 bytes for each 160, 80, 60, 40, 30, 20, 17, 15, 12, 10
//                  MW, 49, 41, 31, 25, 22, 19, 16
const int MEE_BAND_MEM  = 364;

// pushbutton memories 454 .. 478
// 5 bytes each for M1, M2, M3, M4, M5
const int MEE_MEM = 454;


// MEE_DATA1 bits
// bits 0..3 = tune power
const byte MEE_PADDLE_REVERSE = _BV(4);
const byte MEE_TX_EN = _BV(5);
const byte MEE_WINKEY = _BV(6);

// MEE_VFO_x bits
// bits 0 .. 3 tune step
const byte MEE_CW   = _BV(4);
const byte MEE_ATTN = _BV(5);


void radio::reset_mee_eeprom(void) {
  // reset all the EEPROM memory not used by the stock firmware
  for (int i = CW_KEY_TYPE + 1; i < EEPROM.length(); EEPROM.update(i++, 0)); 
}

// read EEPROM settings from setup screens
void radio::read_setup_eeprom(void) {
  byte mee_data1;
  EEPROM.get(USB_CAL,      usbCarrier);
  EEPROM.get(MASTER_CAL,   calibration);
  EEPROM.get(SLOPE_X,      slope_x);
  EEPROM.get(SLOPE_Y,      slope_y);
  EEPROM.get(OFFSET_X,     offset_x);
  EEPROM.get(OFFSET_Y,     offset_y);
  EEPROM.get(CW_SIDETONE,  sidetone_frequency);
  EEPROM.get(CW_SPEED,     cw_millis);
  EEPROM.get(CW_DELAYTIME, cw_delaytime);
  EEPROM.get(CW_KEY_TYPE,  cw_key_type);
  EEPROM.get(MEE_DATA1,    mee_data1);

  // parse MEE_BYTE1
  #ifdef USE_PDL_POL
  paddle_reverse = (mee_data1 & MEE_PADDLE_REVERSE);
  #endif
  #ifdef USE_TX_DIS
  tx_enable = (mee_data1 & MEE_TX_EN);
  #endif
  #ifdef USE_TUNE
  tune_pwr = mee_data1 & 0b1111;
  if (tune_pwr == 0) tune_pwr = 1;
  else if (tune_pwr > 10) tune_pwr = 10;
  #endif

  // set default values (if nothing in EEPROM)
  if (usbCarrier > MAX_USB_CARRIER || usbCarrier < MIN_USB_CARRIER)
    usbCarrier = DEF_USB_CARRIER;
  #ifdef USE_IF_SHIFT
    if_shift_freq = usbCarrier;
  #endif
  if (cw_delaytime < MIN_DELAY || cw_delaytime > MAX_DELAY)
    cw_delaytime = DEF_DELAY;
  if (sidetone_frequency < MIN_CWP || sidetone_frequency > MAX_CWP) 
    sidetone_frequency = DEF_CWP;
  if (cw_millis < MIN_CW_MILLIS || cw_millis > MAX_CW_MILLIS) 
    cw_millis = DEF_CW_MILLIS;
  
  if (cw_key_type > KT_IB)
    cw_key_type = KT_MIN;

  #ifdef USE_WINKEY
    if (mee_data1 & MEE_WINKEY) cw_key_type = KT_WK;
  #endif  
}

// read EEPROM settings from main screen
void radio::read_vfoa(void) {
  byte b;
  long f;
  FREQ_STEP fs;

  EEPROM.get(VFO_A, f);
  vfo_data[false].frequency = (f >= MIN_FREQ || f <= MAX_FREQ) ? f : DEF_VFOA;

  EEPROM.get(VFO_A_MODE, b);
  vfo_data[false].usb = b == MODE_USB;

  EEPROM.get(MEE_VFOA, b);
  fs = (FREQ_STEP) (b & 0b1111);
  vfo_data[false].fs =  (fs > FS_MAX) ? FS_MIN  : fs;
  vfo_data[false].cw = b & MEE_CW;
  vfo_data[false].attn = b & MEE_ATTN;
}

void radio::read_vfob(void) {
  byte b;
  long f;
  FREQ_STEP fs;

  EEPROM.get(VFO_B, f);
  vfo_data[true].frequency = (f >= MIN_FREQ || f <= MAX_FREQ) ? f : DEF_VFOA;

  EEPROM.get(VFO_B_MODE, b);
  vfo_data[true].usb = b == MODE_USB;

  EEPROM.get(MEE_VFOB, b);
  fs = (FREQ_STEP) (b & 0b1111);
  vfo_data[true].fs =  (fs > FS_MAX) ? FS_MIN  : fs;
  vfo_data[true].cw = b & MEE_CW;
  vfo_data[true].attn = b & MEE_ATTN;
}

void radio::read_eeprom(void) {
  read_setup_eeprom();
  read_vfoa();
  read_vfob();
}


void radio::write_setup_eeprom(void) {
  byte mee_data1 = 0
                 #ifdef USE_TX_DIS
                 | (tx_enable ? MEE_TX_EN : 0)
                 #endif
                 #ifdef USE_PDL_POL
                 | (paddle_reverse ? MEE_PADDLE_REVERSE : 0)
                 #endif
                 #ifdef USE_TUNE
                 | tune_pwr
                 #endif
                 #ifdef USE_WINKEY
                 | (cw_key_type == KT_WK ? MEE_WINKEY : 0)
                 #endif
                 ;

  EEPROM.put(USB_CAL,      usbCarrier);
  EEPROM.put(MASTER_CAL,   calibration);
  EEPROM.put(CW_DELAYTIME, cw_delaytime);
  EEPROM.put(SLOPE_X,      slope_x);
  EEPROM.put(SLOPE_Y,      slope_y);
  EEPROM.put(OFFSET_X,     offset_x);
  EEPROM.put(OFFSET_Y,     offset_y);
  EEPROM.put(CW_KEY_TYPE,  (byte)(cw_key_type == MEE_WINKEY ? KT_MIN : cw_key_type));
  EEPROM.put(MEE_DATA1,    mee_data1);
  EEPROM.put(CW_SIDETONE,  sidetone_frequency);
}

void radio::write_vfoa(void) {
  long f;
  byte b;
  f = vfo_data[false].frequency;
  EEPROM.put(VFO_A, f);
  b = vfo_data[false].usb ? MODE_USB : MODE_LSB;
  EEPROM.put(VFO_A_MODE, b);
  EEPROM.put(MEE_VFOA, (byte) (vfo_data[false].fs 
                              | (vfo_data[false].cw ? MEE_CW : 0)
                              | (vfo_data[false].attn ? MEE_ATTN : 0)
                              ));
}

void radio::write_vfob(void) {
  long f;
  byte b;
  f = vfo_data[true].frequency;
  EEPROM.put(VFO_B, f);
  b = vfo_data[true].usb ? MODE_USB : MODE_LSB;
  EEPROM.put(VFO_B_MODE, b);
  EEPROM.put(MEE_VFOB, (byte) (vfo_data[true].fs 
                              | (vfo_data[true].cw ? MEE_CW : 0)
                              | (vfo_data[true].attn ? MEE_ATTN : 0)
                              ));
}

void radio::write_eeprom(void) {
  write_setup_eeprom();
  write_vfoa();
  write_vfob();
}

void radio::restore_settings(void) {
  read_setup_eeprom();
  si5351_set_calibration(calibration);
}

// ********************
// ***** MEMORIES *****
// ********************

#ifdef USE_MEMORY
void radio::write_memory_eeprom(const byte which) {;
  EEPROM.put(MEE_MEM + sizeof(VFO_DATA) * which, vfo_data[using_vfo_b]);
  loop_master::active->update_memory(which);
}

bool radio::check_memory_eeprom(const byte which, VFO_DATA &v) {
  EEPROM.get(MEE_MEM + sizeof(VFO_DATA) * which, v);
  return (v.frequency >= MIN_FREQ && v.frequency <= MAX_FREQ); 
}


void radio::read_memory_eeprom(const byte which) {
  VFO_DATA v;
  if (check_memory_eeprom(which, v)) {
    vfo_data[using_vfo_b] = v;
    calc_step_amount();
    loop_master::active->update_step();
    loop_master::active->update_mode();
    #ifdef USE_ATTN
    loop_master::active->update_attn();
    #endif
    set_frequency(v.frequency);
  }
}
#endif

// ********************
// ***** SET BAND *****
// ********************

#if defined USE_HAM_BAND || defined USE_BC_BAND
bool radio::write_active_vfo_eeprom(RADIO_BAND &rb, int16_t &band_start, int16_t &band_size) {
  if (get_band((int) (vfo_data[using_vfo_b].frequency / 1000L), rb, band_start, band_size)) {
    EEPROM.put(MEE_BAND_MEM + sizeof(VFO_DATA) * (byte) rb, vfo_data[using_vfo_b]);
    return true;
  }
  return false;
}

void radio::write_active_vfo_eeprom(void) {
  RADIO_BAND rb;
  int16_t band_start, band_size;
  write_active_vfo_eeprom(rb, band_start, band_size);
}


bool radio::read_vfo_eeprom(const RADIO_BAND rb, VFO_DATA &vfo_data) {
  EEPROM.get(MEE_BAND_MEM + sizeof(VFO_DATA) * (byte) rb, vfo_data);
  int16_t band_start, band_size, freq_khz;
  band_range(rb, band_start, band_size);
  freq_khz = vfo_data.frequency / 1000L;
  return ((freq_khz >= band_start) && (freq_khz <= (band_start + band_size)));
}
#endif