#include "radio.h"

#include <Arduino.h>

#include "heartbeat.h"
#include "str_buffer.h"
#include "texts.h"
#include "loop_master.h"

// ********************
// ***** 60 meter *****
// ********************
const int STEP_60M = 500;

const byte CWOFS_60M = (byte) (1500 / STEP_60M);
 
const int CHAN_60M[] PROGMEM = {
  (int)(5330500l / STEP_60M),
  (int)(5346500l / STEP_60M),
  (int)(5357000l / STEP_60M),
  (int)(5371500l / STEP_60M),
  (int)(5403500l / STEP_60M)
};

const byte CHAN_CT_60M = sizeof(CHAN_60M) / sizeof(CHAN_60M[0]);

long radio::freq_60m(void) {
  return (long) (pgm_read_word_near(CHAN_60M + (chan_60m - 1))
              + (vfo_data[using_vfo_b].cw ? CWOFS_60M : 0)) * STEP_60M;
}

void radio::set_60m(void) {
  chan_60m = 1;
  vfo_data[using_vfo_b].usb = true;
  loop_master::active->update_mode();
  set_frequency(freq_60m());
}

const int16_t BAND_START[] PROGMEM = {
  // Amateur
  1800, 3500, 5330, 7000, 10100, 14000, 18068, 21000, 24890, 28000,
  // Broadcast
  530, 3200, 4750, 5900, 7300, 9400, 11600, 13570, 15100, 17480
};

const int16_t BAND_SIZE[] PROGMEM = {
  // Amateur
  200, 500, 74, 300, 50, 350, 100, 450, 100, 1700,
  // Broadcast
  1170, 200, 245, 300, 150, 500, 500, 300, 700, 420
};

// const byte BAND_CT = sizeof(BAND_START) / sizeof(BAND_START[0]);

const char * const radio::step_names[] PROGMEM = {
  STEP_10, STEP_20, STEP_50,
  STEP_100, STEP_200, STEP_500,
  STEP_1K, STEP_2K, STEP_5K,
  STEP_10K
};

const char * const radio::keyer_names[] PROGMEM = {
  KEYER_HAND, KEYER_IAMA, 
  #ifdef USE_WINKEY
  KEYER_IAMB, KEYER_WK
  #else
  KEYER_IAMB
  #endif
};

void radio::begin(void) {
  read_eeprom();
  #ifdef USE_AUTO_SAVE
    autosave_freq_change(false);
    autosave_freq_change(true);
  #endif
  calc_step_amount();
  cw_speed = 1200 / cw_millis;
  initPorts();  
  si5351bx_init();
  si5351_set_calibration(calibration);
  #ifdef USE_ATTN
  set_attn();
  #endif
  setHWFrequencyRX();
}

void radio::clear_setup_highlights(void) {
  #ifdef USE_TUNE
  in_tune_pwr =
  #endif
  in_cw_pitch =
  in_cw_delay =
  in_freq_cal =
  in_bfo_cal = 
  #ifdef USE_RF_SHIFT
  in_rf_shift = 
  #endif
  false;
  sidetone(false);
}
 
long radio::adj_range(const long value, const bool inc, const long step, const long minimum, const long maximum) {
  long new_value = value;
  new_value += inc ? step : -step;
  if (new_value < minimum) new_value = minimum;
  else if (new_value > maximum) new_value = maximum;
  return new_value;
 }

void radio::adj_frequency(const bool inc) {
  #ifdef USE_LOCK
  if (!lock) {
  #endif
    if (chan_60m) {
      chan_60m += inc ? 1 : -1;
      if (chan_60m == 0) chan_60m = CHAN_CT_60M;
      else if (chan_60m > CHAN_CT_60M) chan_60m = 1;
    }  
    vfo_data[using_vfo_b].frequency = chan_60m
      ? freq_60m()
      : adj_range(vfo_data[using_vfo_b].frequency, inc, step_amount, MIN_FREQ, MAX_FREQ);
    loop_master::active->update_vfo(using_vfo_b);
    #ifdef USE_AUTO_SAVE
    autosave_freq_change(using_vfo_b);
    #endif
    setHWFrequencyRX();
  #ifdef USE_LOCK
  }
  #endif
}

void radio::calc_step_amount(void) {
  byte freq_step = (byte) vfo_data[using_vfo_b].fs;
  step_amount = 10;
  for (byte i=0; i < freq_step / 3; i++)
    step_amount *= 10;
    switch (freq_step % 3) {
      case 1 : step_amount *= 2; break;
      case 2 : step_amount *= 5; break;
  }
}

void radio::inc_step(const bool reset) {
  FREQ_STEP f = vfo_data[using_vfo_b].fs; 
  set_step((reset || (f == FS_MAX)) ? FS_MIN : (FREQ_STEP)(f + 1));
}

void radio::set_step(const FREQ_STEP step) {
  vfo_data[using_vfo_b].fs = step;
  calc_step_amount();
  vfo_data[using_vfo_b].frequency -= vfo_data[using_vfo_b].frequency % step_amount;
  rit_amount -= (rit_amount % step_amount);
  loop_master::active->update_vfo(using_vfo_b);
  loop_master::active->update_step();
  setHWFrequencyRX();
}

void radio::toggle_cw(void) {
  vfo_data[using_vfo_b].cw = !vfo_data[using_vfo_b].cw;
  loop_master::active->update_mode();
  if (chan_60m) {
    vfo_data[using_vfo_b].frequency = freq_60m();
    loop_master::active->update_vfo(using_vfo_b);
  }
  setHWFrequencyRX();
}

void radio::toggle_cw(const bool on) {
  vfo_data[using_vfo_b].cw = on;
  loop_master::active->update_mode();
  setHWFrequencyRX();
}

void radio::toggle_usb(void) { 
  vfo_data[using_vfo_b].usb = !vfo_data[using_vfo_b].usb;
  loop_master::active->update_mode();
  setHWFrequencyRX();
}

void radio::toggle_usb(const bool usb) {
  vfo_data[using_vfo_b].usb = usb;
  loop_master::active->update_mode();
  setHWFrequencyRX();
}

void radio::update_both_vfos(void) {
  loop_master::active->update_vfo(false);
  loop_master::active->update_vfo(true);
}

void radio::eq_vfo(const bool to_vfob) {
  vfo_data[!to_vfob] = vfo_data[to_vfob];
  #ifdef USE_AUTO_SAVE
  autosave_freq_change(!to_vfob);
  #endif
  calc_step_amount();
  update_both_vfos();
  #ifdef USE_ATTN
    set_attn();
    loop_master::active->update_attn();
  #endif
  setHWFrequencyRX();
  loop_master::active->update_step();
  loop_master::active->update_mode();
}

long radio::adjusted_frequency(const bool vfob) {
  return vfo_data[vfob].frequency + ((rit && !in_tx) ? rit_amount : 0);
}

VFO_STATE radio::vfo_state(const bool vfob) {
  return (using_vfo_b == vfob) ? (split ? (in_tx ? VS_INACTIVE_RX : VS_ACTIVE_RX) : (in_tx ? VS_ACTIVE_TX : VS_ACTIVE_RX))
                               : (split ? (in_tx ? VS_ACTIVE_TX : VS_INACTIVE_TX) : VS_INACTIVE_RX);
}

#ifdef USE_ATTN
void radio::set_attn(void) {
  si5351bx_drive[2] = ((
    #ifdef USE_TUNE
    tune || 
    #endif
    cw_tx || (!in_tx && vfo_data[using_vfo_b].attn)) ? 0 : 3);
}
#endif

void radio::set_ptt(const bool on) {
  ptt = in_tx = on;
  #ifdef USE_ATTN
  set_attn();
  #endif
  if (on) {
    setHWFrequencyTX();
    #ifndef USE_RX_ONLY
      #ifdef USE_TX_DIS
      digitalWrite(TX_RX, tx_enable);  
      #else
      digitalWrite(TX_RX, true);
      #endif
    #endif
  } else {
    digitalWrite(TX_RX, 0);
    setHWFrequencyRX();
  }
  update_both_vfos();
}  

void radio::set_cw_tx(const bool on) {
  in_tx = cw_tx = on;
  
  digitalWrite(TX_RX, 0);
  digitalWrite(CW_KEY, 0);  
}  


void radio::set_rit(const bool on) {
  rit = on;
  setHWFrequencyRX();
}

bool radio::set_frequency(const long freq) 
{
  if (freq < MIN_FREQ || freq > MAX_FREQ || in_tx)
    return false;
  else {
    #ifdef USE_AUTO_SAVE
    autosave_freq_change(using_vfo_b);
    #endif
    rit_amount = 0;
    rit = false;
    vfo_data[using_vfo_b].frequency = freq - freq % step_amount;
  
    loop_master::active->update_vfo(using_vfo_b);
    setHWFrequencyRX();    
    loop_master::active->update_rit();
    return true;
  }
}

bool radio::set_khz(const long kHz) {
  #if defined USE_HAM_BAND || defined USE_BC_BAND 
  write_active_vfo_eeprom();
  #endif
  chan_60m = false;
  return set_frequency(1000 * kHz);
}



void radio::band_range(const RADIO_BAND rb, int16_t &band_start, int16_t &band_size) {
  band_start = pgm_read_word_near(BAND_START + (byte) rb);
  band_size = pgm_read_word_near(BAND_SIZE + (byte) rb);
}

bool radio::get_band(const int16_t freq_khz, RADIO_BAND &rb, int16_t &band_start, int16_t &band_size) {
  for (rb = RB_MIN; rb <= RB_MAX; rb = (RADIO_BAND)((byte) rb + 1)) {
    band_range(rb, band_start, band_size);
    int16_t t_offset = freq_khz - band_start;
    if ((t_offset >= 0) && (t_offset <= band_size)) return true;
  }
  return false;
}

#if defined USE_HAM_BAND || defined USE_BC_BAND
void radio::set_band(const RADIO_BAND band) {
  RADIO_BAND old_rb;
  int16_t old_band_start, new_band_start, old_band_size, new_band_size;
  int16_t temp_freq = 0;
  int16_t freq_khz = (int16_t) (vfo_data[using_vfo_b].frequency / 1000L);
  chan_60m = 0;
  #if defined USE_HAM_BAND || defined USE_BC_BAND 
  bool have_old = write_active_vfo_eeprom(old_rb, old_band_start, old_band_size);
  #endif

  band_range(band, new_band_start, new_band_size);
  VFO_DATA vfo;
  if (read_vfo_eeprom(band, vfo)) {
    vfo_data[using_vfo_b] = vfo;
    calc_step_amount();
    loop_master::active->update_step();
    loop_master::active->update_mode();
    #ifdef USE_ATTN
    loop_master::active->update_attn();
    #endif
    set_frequency(vfo.frequency);
  } else {
    // if nothing in memory, set to center of band
    if (have_old) {
      if (old_rb != band)
        temp_freq = (new_band_start + ((long) (freq_khz - old_band_start) * new_band_size / old_band_size));
    } else {
      temp_freq = new_band_start + (new_band_size >> 1);
    }
    if (temp_freq) {
      if (!vfo_data[using_vfo_b].cw)
        toggle_usb(temp_freq > 10000);
      set_frequency(temp_freq * 1000L);
    }
  }  
}
#endif

#if 1
void radio::setTXFilters(long freq) {
  const byte filter_su[] = {_BV(TX_LPF_C), _BV(TX_LPF_B), _BV(TX_LPF_A), 0};
  PORTD = (PORTD & ~(_BV(TX_LPF_A) | _BV(TX_LPF_B) | _BV(TX_LPF_C))) | filter_su[freq / 7000000l];
}
#else
// stock code
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

void radio::setTXFilters(long freq){
  
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
#endif

#ifdef USE_ATTN
void radio::toggle_attn(void) 
{ 
  vfo_data[using_vfo_b].attn = !vfo_data[using_vfo_b].attn;
  set_attn();
  setHWFrequencyRX();
  loop_master::active->update_attn();
}
#endif


void radio::initPorts(void) {
  pinMode(TX_RX, OUTPUT);
  digitalWrite(TX_RX, 0);

  pinMode(TX_LPF_A, OUTPUT);
  digitalWrite(TX_LPF_A, 0);

  pinMode(TX_LPF_B, OUTPUT);
  digitalWrite(TX_LPF_B, 0);

  pinMode(TX_LPF_C, OUTPUT);
  digitalWrite(TX_LPF_C, 0);

  pinMode(CW_KEY, OUTPUT);
  digitalWrite(CW_KEY, 0);
}

void radio::set_active_vfob(const bool do_vfob) {
  using_vfo_b = do_vfob;
  calc_step_amount();
  update_both_vfos();
  loop_master::active->update_mode();
  loop_master::active->update_step();
  #ifdef USE_ATTN
  loop_master::active->update_attn();
  set_attn();
  #endif
  setHWFrequencyRX();
}

void radio::toggle_active_vfo(void) {
  set_active_vfob(!using_vfo_b);
}

const char * const radio::mode_name(void) {
  return vfo_data[using_vfo_b].cw ? (vfo_data[using_vfo_b].usb ? CWU_TEXT : CWL_TEXT)
                                  : (vfo_data[using_vfo_b].usb ? USB_TEXT : LSB_TEXT);
}

const char * const radio::step_name(void)  {
  return (char *)pgm_read_word(&step_names[vfo_data[using_vfo_b].fs]);
};

void radio::toggle_split(void) {
  split = !split;
  update_both_vfos();
  loop_master::active->update_split();
}

void radio::toggle_split(const bool on) {
  split = !on;
  toggle_split();
}

#ifdef USE_SPOT
void radio::toggle_spot(void) {
  spot = !spot;
  sidetone(spot);
  loop_master::active->update_spot();
}
#endif

#ifdef USE_LOCK
void radio::toggle_lock(const bool on) {
  lock = on;
  loop_master::active->update_lock();
}

void radio::toggle_lock(void) {
  lock = !lock;
  loop_master::active->update_lock();
}
#endif

// use direct hardware I/O. Much smaller than using tone()
// direct hw output - no ISR needed
void radio::sidetone(const bool on) {
  if (on) {
    OCR0A  = (byte)(F_CPU / (256l * radio_obj.sidetone_frequency * 2) - 1);
    TCCR0A |= _BV(COM0A0);  // toggle OC0A on compare match
  } else {
    TCCR0A &= ~_BV(COM0A0);  // stop toggling OC0A
    digitalWrite(CW_TONE, 0);
  }
}

void radio::toggle_rit(const bool reset) {
  if (reset) {
    rit = false;
    rit_amount = 0;
  } else
    rit = !rit;
  rotary_mode = rit ? RM_RIT : RM_VFO;
  setHWFrequencyRX();  
  loop_master::active->update_rit();
  update_both_vfos();
}

void radio::adj_rit(const bool inc) {
  rit_amount = adj_range(rit_amount, inc, step_amount, -RIT_LIMIT, RIT_LIMIT);
  setHWFrequencyRX();  
  update_both_vfos();
}


#ifdef USE_PDL_POL
void radio::toggle_paddle_polarity(void) {
  paddle_reverse = !paddle_reverse;
  loop_master::active->update_paddle_polarity();
}
#endif

void radio::inc_keyer(void) {
  cw_key_type = ((cw_key_type == KT_MAX) ? KT_MIN : (KEYER_TYPE)(cw_key_type + 1));
  loop_master::active->update_keyer_mode();
}

#ifdef USE_TX_DIS
void radio::toggle_tx_disable(void) {
  tx_enable = !tx_enable;
  loop_master::active->update_tx_disable();
}
#endif

#ifdef USE_AUTO_SAVE
void radio::autosave_freq_change(const bool vfob) {
  vfo_autosave[vfob].last_change = millis();
  vfo_autosave[vfob].need_save = true;
}

void radio::check_autosave(void) {
  unsigned long m = millis();
  if (vfo_autosave[false].need_save && ((m - vfo_autosave[false].need_save) > AUTOSAVE_HOLD)) {
    vfo_autosave[false].need_save = false;
    write_vfoa();
  }
  if (vfo_autosave[true].need_save && ((m - vfo_autosave[true].need_save) > AUTOSAVE_HOLD)) {
    vfo_autosave[true].need_save = false;
    write_vfob();
  }
}
#endif

#ifdef USE_RF_SHIFT_STEP
const byte RF_SHIFT_STEPS[] PROGMEM = {21, 41, 83};  // {-22, -42, -74}
void radio::inc_rf_shift_step(const bool reset) {
  if (reset) rf_attn_step = 0;
  else {
    rf_attn_step++;
    if (rf_attn_step > sizeof(RF_SHIFT_STEPS)) rf_attn_step = 0;
  }
  rf_shift_freq = firstIF + (rf_attn_step ? pgm_read_byte_near(RF_SHIFT_STEPS + rf_attn_step - 1) * 1000L : 0);
  setHWFrequencyRX();
  loop_master::active->update_rf_shift_step();
}
#endif