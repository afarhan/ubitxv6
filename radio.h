#pragma once

#include <Arduino.h>
#include "ubitx_si5351.h"
#include "defines.h"
#include "pins.h"
#include "texts.h"
#include "parameter_limits.h"

const byte MODE_USB = 3;
const byte MODE_LSB = 2;

const long firstIF = 45000000L; // 45005000L;


// keyer paddle analog values
const uint16_t PS_BOTH_MAX  = 305;   // BOTH
const uint16_t PS_RIGHT_MAX = 512;   // TIP 
const uint16_t PS_LEFT_MAX  = 860;   // RING

const byte CW_DELAY_MULTIPLE = 10; 

enum RADIO_BAND {RB_MIN, RB_160=RB_MIN, RB_80, RB_60, RB_40, RB_30, RB_20, RB_17, RB_15, RB_12, RB_10,
                 RB_MW, RB_90, RB_B60, RB_49, RB_41, RB_31, RB_25, RB_22, RB_19, RB_16, RB_MAX=RB_16};



enum FREQ_STEP {FS_MIN, FS_10=0, FS_20, FS_50,
                FS_100, FS_200, FS_500,
                FS_1K, FS_2K, FS_5K, FS_10K,
                FS_MAX=FS_10K};


enum KEYER_TYPE {KT_MIN, KT_HAND=0, KT_IA, KT_IB, 
  #ifdef USE_WINKEY
  KT_WK, KT_MAX=KT_WK
  #else
  KT_MAX=KT_IB
  #endif
  }; 
  
enum VFO_STATE {VS_INACTIVE_RX, VS_ACTIVE_RX, VS_INACTIVE_TX, VS_ACTIVE_TX};

enum ROTARY_MODE {RM_NONE, RM_VFO, RM_CWS, RM_CWP, RM_IF_SHIFT, RM_RIT,
                  RM_BFO, RM_FREQ, RM_DELAY, RM_TUNE_PWR, RM_RF_SHIFT};

struct VFO_DATA {
  long frequency : 29;
  bool usb       :  1;
  bool cw        :  1;
  bool attn      :  1;
  FREQ_STEP fs   :  4;
};

struct VFO_AUTOSAVE {
  unsigned long last_change;
  bool need_save;
};

class radio {
  private:
    // -----
    static byte chan_60m;
    static int16_t rit_amount;
    static uint16_t step_amount;

    // *******************
    // ***** Methods *****
    // *******************

    #ifdef USE_ATTN
    static void set_attn(void);
    #endif
    static long adj_range(const long value, const bool inc, const long step, const long minimum, const long maximum);
    static void calc_step_amount(void);
    static void initPorts(void);
    static void setTXFilters(long freq);
    static void setHWFrequencyRX(void);
    static void setHWFrequencyTX(void);
    static void update_both_vfos(void);
    static void band_range(const RADIO_BAND rb, int16_t &band_start, int16_t &band_size);
    static bool get_band(const int16_t freq_khz, RADIO_BAND &rb, int16_t &band_start, int16_t &band_size);
    static const char * const step_names[];
    static const char * const keyer_names[];


  public:
    static VFO_DATA vfo_data[];

    #ifdef USE_AUTO_SAVE
    static VFO_AUTOSAVE vfo_autosave[];
    static void autosave_freq_change(const bool vfob);
    static void check_autosave(void);
    #endif

    static bool rit, ptt, in_tx, cw_tx;
    #ifdef USE_PDL_POL
    static bool paddle_reverse;
    #endif
    #ifdef USE_TUNE
    static bool tune;
    static byte tune_pwr;  // 1 .. 10 ==> 10 .. 100%
    static bool in_tune_pwr;
    #endif
    #ifdef USE_SPOT
    static bool spot;
    #endif
    #ifdef USE_LOCK
    static bool lock;
    #endif
    static bool in_cw_speed, in_cw_pitch, in_freq_cal, in_bfo_cal, in_cw_delay;
    #ifdef USE_IF_SHIFT
    static bool in_if_shift;
    static long if_shift_freq;
    #endif
    static int slope_x;
    static int slope_y;
    static int offset_x;
    static int offset_y;
    static byte cw_speed;
    static ROTARY_MODE rotary_mode;
    static int cw_delaytime;
    static bool using_vfo_b; 
    static long sidetone_frequency;
    static KEYER_TYPE cw_key_type;
    #ifdef USE_TX_DIS
    static bool tx_enable;
    #endif
    static bool split;
    static long usbCarrier;
    static long calibration;
    static int cw_millis;

    // *******************
    // ***** Methods *****
    // *******************

    static void begin(void);

    static void reset_mee_eeprom(void);
    static void read_eeprom(void);
    #if defined USE_HAM_BAND || defined USE_BC_BAND
    static bool write_active_vfo_eeprom(RADIO_BAND &rb, int16_t &band_start, int16_t &band_size);
    static void write_active_vfo_eeprom(void);
    static bool read_vfo_eeprom(const RADIO_BAND rb, VFO_DATA &vfo_data);
    static void set_band(const RADIO_BAND band);
    #endif
    static void read_setup_eeprom(void);
    static void read_vfoa(void);
    static void write_vfoa(void);
    static void read_vfob(void);
    static void write_vfob(void);
    static void write_eeprom(void);
    static void write_setup_eeprom(void);
    static void restore_settings(void);
    static void clear_setup_highlights(void);
    #ifdef USE_MEMORY
    static void write_memory_eeprom(const byte which);
    static void read_memory_eeprom(const byte which);
    static bool check_memory_eeprom(const byte which, VFO_DATA &v);
    #endif

    static void toggle_cw(void);
    static void toggle_cw(const bool on);

    static void toggle_usb(void);
    static void toggle_usb(const bool usb);

    static void adj_frequency(const bool inc);
    static void inc_step(const bool reset);
    static void set_step(const FREQ_STEP step);
    static void adj_cw_speed(const bool inc);
    static void adj_cw_pitch(const bool inc);
    static void adj_rit(const bool inc);

    static void eq_vfo(const bool to_vfob);

    static void set_cw_tx(const bool on);
    static void set_ptt(const bool on);
    static bool set_frequency(const long frequency);
    static bool set_khz(const long kHz);
    static void set_rit(const bool on);

  
    #ifdef USE_ATTN
    static void toggle_attn(void);
    #endif

    static long adjusted_frequency(const bool vfob);
    static VFO_STATE vfo_state(const bool vfob);

    static void set_active_vfob(const bool);
    static void toggle_active_vfo(void);

    static void set_cw_oscillators(void);

    static void toggle_split(void);
    static void toggle_split(const bool on);

    #ifdef USE_TUNE
    static void toggle_tune(void);
    #endif
    #ifdef USE_SPOT
    static void toggle_spot(void);
    #endif

    static void toggle_lock(void);
    static void toggle_lock(const bool on);

    static void toggle_rit(const bool reset);

    #ifdef USE_IF_SHIFT
    static void toggle_if_shift(const bool reset);
    static void adj_if_shift(const bool inc);
    #endif

    static void toggle_cw_speed(void);
    static void toggle_cw_pitch(void);
    #ifdef USE_PDL_POL
    static void toggle_paddle_polarity(void);
    #endif
    static void inc_keyer(void);
    #ifdef USE_TX_DIS
    static void toggle_tx_disable(void);
    #endif

    static void toggle_cw_delay(void);
    static void adj_cw_delay(const bool inc);

    static void toggle_freq_cal(void);
    static void adj_master_cal(const bool inc);

    static void toggle_bfo_cal(void);
    static void adj_bfo(const bool inc);

    static void toggle_tune_pwr(void);
    static void adj_tune_pwr(const bool inc);

    static long freq_60m(void);
    static void set_60m(void);

    #ifdef USE_RF_SHIFT
      static long rf_shift_freq;
      static bool in_rf_shift;
      static void toggle_rf_shift(const bool reset);
      static void adj_rf_shift(const bool inc);
    #endif

    #ifdef USE_RF_SHIFT_STEP
      static long rf_shift_freq;
      static byte rf_attn_step;
      static void inc_rf_shift_step(const bool reset);
    #endif

    static void rotary(const bool inc);
    static void sidetone(const bool on);

    static const char * const mode_name(void); 
    static const char * const step_name(void);
    static const char * const keyer_name(void) {return (char *)pgm_read_word(&keyer_names[cw_key_type]);};
    #ifdef USE_PDL_POL
    static const char * const paddle_polarity(void) {return (paddle_reverse ? REVERSE : NORMAL);};
    #endif
};

extern radio radio_obj;