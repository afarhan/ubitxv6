#pragma once

#include <Arduino.h>
#include "defines.h"

enum MINI_LABEL {ML_PTT};

extern const unsigned RX_VFO_COLOR;
extern const unsigned TX_VFO_COLOR;
extern const unsigned INACTIVE_RX_VFO_COLOR;
extern const unsigned INACTIVE_TX_VFO_COLOR;
extern const unsigned BUTTON_BORDER_COLOR;
extern const unsigned BUTTON_BUTTON_BG_COLOR;
extern const unsigned BUTTON_TEXT_COLOR;
extern const unsigned HIGH_BUTTON_BG_COLOR;
extern const unsigned HIGH_BUTTON_TEXT_COLOR;
extern const unsigned BG_COLOR;
extern const unsigned PTT_COLOR;
extern const unsigned INFO_TEXT_COLOR;
extern const unsigned INFO_BORDER_COLOR;
extern const unsigned INFO_BG_COLOR;
extern const unsigned KEYPAD_COLOR;
extern const unsigned MINI_LABEL_COLOR;

extern const unsigned BANNER_BG;
extern const unsigned BANNER_FG;
const unsigned BANNER_SIZE = 2;

const byte CHAR_HEIGHT = 8;
const byte CHAR_WIDTH = 6;

extern const unsigned CROSS_FG;
extern const byte TC_TEXT_SIZE;
const unsigned CROSS_SIZE = 5;

const unsigned SCREEN_WIDTH = 320;  // display width, in pixels
const unsigned SCREEN_HEIGHT = 240; // display height, in pixels

const byte BUTTON_WIDTH = 64;
const byte BUTTON_HEIGHT = 40;

const byte INFO_TEXT_SIZE = 2;

extern const byte VFOA_LEFT;
extern const byte VFOA_TOP;
extern const byte VFOA_SIZE;
extern const byte VFOB_SIZE;
extern const byte VFOB_TOP;
extern const byte VFOB_LEFT;

const byte MAX_BUTTONS = (SCREEN_WIDTH / BUTTON_WIDTH) * (SCREEN_HEIGHT / BUTTON_HEIGHT);

class display {
  void button_left_top(const byte which, uint16_t &b_left, byte &b_top);

  byte info_top;
  uint16_t info_left, info_wide;

  uint16_t info_bg_color;

  static long current_vfoa, current_vfob, current_info;
  static uint16_t color_vfoa, color_vfob;

  // void do_digit(void);
  void display_digits(long _new_value, long old_value, 
    uint16_t left, byte top, uint16_t fg_color, uint16_t bg_color, byte font_size,
    const byte len, const bool do_negative);

  static const char * const mini_label[];

  void display_vfo(long new_value, long old_value,
                   const uint16_t new_color, const uint16_t old_color,
                  const int16_t x, const int8_t y, const int8_t textsize);

  public:
    display();

    const char *banner_override;

    void begin(void);

    void banner(void);

    void int_to_str_buffer(char *buffer, uint16_t value);
    #ifdef USE_MEMORY
    void mem_button(byte which);
    #endif
    
    void draw_button_border(const byte which);
    void highlight_clear_button(const byte which, const bool highlight, const byte text_size=2, const bool two_line=false);
    void set_button_texts(const char *line1, const char *line2);  

    void draw_info_border(const byte button, const byte wide);
    void set_info_border(const byte button, const byte wide);
    void update_info_number(const long new_value,
                            const byte len, const byte font_size, 
                            const bool do_negative=false);

    void center_text(const byte text_size, const uint16_t color);

    void cross(const int16_t x, const int16_t y, const bool erase);

    void update_keypad_entry(const long new_value);

    void display_mini_label(MINI_LABEL ml, const bool show);

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillScreen(uint16_t color);

    void reset_persistants(void);

    void display_vfoa(const bool clear);
    void display_vfob(const bool clear);

    void display_cw_speed(const bool clear);
    #ifdef USE_IF_SHIFT
    void display_if_shift(const bool clear);
    #endif

    void display_cw_pitch(const bool clear);
    void display_cw_delay(const bool clear);
    void display_freq_cal(const bool clear);
    void display_bfo_cal(const bool clear);
    #ifdef USE_TUNE
    void display_tune_pwr(const bool clear);
    #endif
    void display_rf_shift(const bool clear);

    #ifdef USE_HB_DISP
    void heartbeat(void);
    #endif
    #ifdef USE_CAT_ACTIVITY
    void cat_activity(void);
    #endif
};

extern display disp;