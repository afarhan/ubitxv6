#include "display.h"
#include <SPI.h>

#include "radio.h"
#include "str_buffer.h"
#include "defines.h"
#include "texts.h"
#include "PDQ_MinLib/PDQ_ILI9341_config.h"
#include "PDQ_MinLib/PDQ_ILI9341.h"
#include "nano_font.h"

PDQ_ILI9341 tft;

const byte BUTTON_RADIUS = 2;

const byte KEYPAD_LEFT = 96;
const byte KEYPAD_TOP = 40;
const byte KEYPAD_SIZE = 3;

const byte MINI_LABEL_SIZE = 1;
const byte MINI_LABEL_TOP = 48;
const byte MINI_LABEL_LEFT = 67;

const byte TC_TEXT_SIZE = 2;

const byte RIT_DIGITS    = 6;
const byte KEYPAD_DIGITS = 5;

const byte VFOA_LEFT = BUTTON_WIDTH;
const byte VFOA_TOP = BUTTON_HEIGHT + (BUTTON_HEIGHT - VFOA_SIZE * CHAR_HEIGHT) / 2;
const byte VFOA_SIZE = 3;

const byte VFOB_LEFT = BUTTON_WIDTH * 3;
const byte VFOB_TOP = VFOA_TOP;
const byte VFOB_SIZE = VFOA_SIZE;

const char * const display::mini_label[] PROGMEM = {PTT_TEXT};

const byte CURRENT_MODE_EXISTS = _BV(0);
const byte CURRENT_MODE_USB    = _BV(1);
const byte CURRENT_MODE_CW     = _BV(2);

// TFT tft;

#define RGB565(r,g,b) ((((r) & 0b11111000) << 8) | (((g) & 0b11111100) << 3) | (((b) >> 3))) 

#if 0
// Adafruit colors
const uint16_t ILI9341_BLACK = 0x0000;       ///<   0,   0,   0
const uint16_t ILI9341_NAVY = 0x000F;        ///<   0,   0, 123
const uint16_t ILI9341_DARKGREEN = 0x03E0;   ///<   0, 125,   0
const uint16_t ILI9341_DARKCYAN = 0x03EF;    ///<   0, 125, 123
const uint16_t ILI9341_MAROON = 0x7800;      ///< 123,   0,   0
const uint16_t ILI9341_PURPLE = 0x780F;      ///< 123,   0, 123
const uint16_t ILI9341_OLIVE = 0x7BE0;       ///< 123, 125,   0
const uint16_t ILI9341_LIGHTGREY = 0xC618;   ///< 198, 195, 198
const uint16_t ILI9341_DARKGREY = 0x7BEF;    ///< 123, 125, 123
const uint16_t ILI9341_BLUE = 0x001F;        ///<   0,   0, 255
const uint16_t ILI9341_GREEN = 0x07E0;       ///<   0, 255,   0
const uint16_t ILI9341_CYAN = 0x07FF;        ///<   0, 255, 255
const uint16_t ILI9341_RED = 0xF800;         ///< 255,   0,   0
const uint16_t ILI9341_MAGENTA = 0xF81F;     ///< 255,   0, 255
const uint16_t ILI9341_YELLOW = 0xFFE0;      ///< 255, 255,   0
const uint16_t ILI9341_WHITE = 0xFFFF;       ///< 255, 255, 255
const uint16_t ILI9341_ORANGE = 0xFD20;      ///< 255, 165,   0
const uint16_t ILI9341_GREENYELLOW = 0xAFE5; ///< 173, 255,  41
const uint16_t ILI9341_PINK = 0xFC18;        ///< 255, 130, 198
#endif

// in Adafruit GFX, but not PDQ
const unsigned ILI9341_OLIVE = RGB565(123, 125, 0);
const unsigned ILI9341_DARKGREEN = RGB565(0, 125, 0);

const unsigned RX_VFO_COLOR = RGB565(255,255,0);
const unsigned TX_VFO_COLOR = RGB565(255,  0,0);
const unsigned INACTIVE_RX_VFO_COLOR = RGB565(63, 63,  0);
const unsigned INACTIVE_TX_VFO_COLOR = RGB565(63, 0, 0);


const unsigned BUTTON_BORDER_COLOR = ILI9341_OLIVE;
const unsigned BUTTON_BUTTON_BG_COLOR = ILI9341_BLUE;
const unsigned BUTTON_TEXT_COLOR = ILI9341_CYAN;
const unsigned HIGH_BUTTON_BG_COLOR = ILI9341_CYAN;
const unsigned HIGH_BUTTON_TEXT_COLOR = ILI9341_BLACK;
const unsigned BG_COLOR = ILI9341_BLACK;
const unsigned PTT_COLOR = ILI9341_RED;
const unsigned INFO_TEXT_COLOR = ILI9341_CYAN;
const unsigned INFO_BORDER_COLOR = ILI9341_BLUE;
const unsigned INFO_BG_COLOR = RGB565(31, 31, 0);
const unsigned KEYPAD_COLOR = ILI9341_CYAN;
const unsigned MINI_LABEL_COLOR = ILI9341_RED;

const unsigned HB_COLOR = ILI9341_YELLOW;
const unsigned CAT_HB_COLOR = ILI9341_CYAN;

const unsigned BANNER_BG = ILI9341_DARKGREEN;
const unsigned BANNER_FG = ILI9341_BLACK;

const unsigned CROSS_FG = ILI9341_WHITE;

long display::current_vfoa;
long display::current_vfob;
long display::current_info;
uint16_t display::color_vfoa;
uint16_t display::color_vfob;

display::display() {
  banner_override = NULL;
}  

void display::begin(void) {  
  tft.begin();
  // tft.setFont(ubitx_font);
  tft.setFont(NULL);
  tft.setRotation(1); 
  tft.fillScreen(BG_COLOR);  
} 

void display::banner(void) {
  tft.fillRect(0, 0, SCREEN_WIDTH, BANNER_SIZE * CHAR_HEIGHT, BANNER_BG);
  strcpy_P(str_buffer1, banner_override ? banner_override : BANNER);
  banner_override = NULL;
  tft.setCursor ((SCREEN_WIDTH - strlen(str_buffer1) * CHAR_WIDTH * BANNER_SIZE) / 2 + 1, BANNER_SIZE / 2);
  tft.setTextColor(BANNER_FG);
  tft.setTextSize(BANNER_SIZE);
  tft.print(str_buffer1);
}


void display::display_digits(long new_value, long old_value, 
  uint16_t left, byte top, uint16_t fg_color, uint16_t bg_color, byte font_size,
  const byte len, const bool do_negative) {
  bool erase_old = (old_value != __LONG_MAX__);
  bool draw_new =  (new_value != __LONG_MAX__);
  bool first_zero = true;
  bool old_minus, new_minus;
  uint16_t x = left +  font_size * CHAR_WIDTH * (len-1);
  byte j = len;

  if (erase_old && (old_value < 0)) {
    old_minus = true;
    old_value = -old_value;
  } else
    old_minus = false;

  if (draw_new && (new_value < 0)) {
    new_minus = true;
    new_value = -new_value;
  } else
    new_minus = false;
  
  if (do_negative)  j++;   // j--

  for (byte i=0; i<j; i++) {
    unsigned char old_digit, new_digit;

    old_digit = new_digit = 0;
    if (erase_old) {
      old_digit = old_value % 10;
      old_value /= 10;
      if ((old_digit || old_value || first_zero)) 
        old_digit += '0';
      else if (old_minus) {
        old_digit = '-';
        old_minus = false;
      }
    } 
    if (draw_new) {
      new_digit = new_value % 10;
      new_value /= 10;
      if (new_digit || new_value || first_zero) 
        new_digit += '0';
      else if (new_minus) {
        new_digit = '-';
        new_minus = false;
      }
    }

    if ((old_digit != new_digit)) {
      if (old_digit) tft.drawChar(x, top, old_digit, bg_color, bg_color, font_size);  
      if (new_digit) tft.drawChar(x, top, new_digit, fg_color, fg_color, font_size);
    }
    x -= font_size * CHAR_WIDTH;

    first_zero = false;
  }
}


void display::update_info_number(const long new_value, 
                                 const byte len, const byte font_size,
                                 const bool do_negative=false) {
  
  display_digits(new_value, current_info, 
    info_left + (info_wide - CHAR_WIDTH * font_size * len) / 2,
    info_top + (BUTTON_HEIGHT - CHAR_HEIGHT * font_size) / 2,
    INFO_TEXT_COLOR, info_bg_color, font_size,
    len, do_negative);

  current_info = new_value;
}



void display::int_to_str_buffer(char *buffer, uint16_t value) {
  if (value) {
    byte digits[8];
    bool do_zero = false;
    for (int8_t i=7; i>=0; i--) {
      digits[i] = value % 10;
      value /= 10; 
    }
    for (int8_t i=0; i<8; i++) {
      if (digits[i] || do_zero) {
        *buffer++ = digits[i] + '0';
        do_zero = true;
      }
    }
  } else {
    *buffer++ = '0';
  }
  *buffer = '\0';
}

#ifdef USE_MEMORY
void display::mem_button(byte which) {
  VFO_DATA v;
  str_buffer1[0] = 'M';
  str_buffer1[1] = '0' + which;
  str_buffer1[2] = '\0';
  if (radio_obj.check_memory_eeprom(which - 1, v))
    int_to_str_buffer(str_buffer2, (int) (v.frequency / 1000L));
  else
   strcpy_P(str_buffer2, STARS);
}
#endif

void display::button_left_top(const byte which, uint16_t &b_left, byte &b_top) {
  b_top = which / (SCREEN_WIDTH / BUTTON_WIDTH) * BUTTON_HEIGHT;
  b_left = (which % (SCREEN_WIDTH / BUTTON_WIDTH)) * BUTTON_WIDTH;
}

void display::draw_button_border(const byte which) {
  byte b_top;
  uint16_t b_left;
  button_left_top(which, b_left, b_top);
  tft.fillRect(b_left+1, b_top+1, BUTTON_WIDTH-2, BUTTON_HEIGHT-2, BG_COLOR);
  tft.drawRoundRect(b_left+2, b_top+2, BUTTON_WIDTH-4, BUTTON_HEIGHT-4, BUTTON_RADIUS, BUTTON_BORDER_COLOR);
}

void display::set_button_texts(const char *line1, const char *line2) {
  strcpy_P(str_buffer1, line1);
  strcpy_P(str_buffer2, line2);
}

void display::highlight_clear_button(const byte which, const bool highlight, const byte text_size=2, const bool two_line=false) {
  byte b_top;
  uint16_t b_left, b_text_top;
  button_left_top(which, b_left, b_top);
  tft.fillRoundRect(b_left+3, b_top+3, BUTTON_WIDTH-6, BUTTON_HEIGHT-6, BUTTON_RADIUS, highlight ? HIGH_BUTTON_BG_COLOR : BUTTON_BUTTON_BG_COLOR);
  tft.setTextSize(text_size);
  tft.setTextColor(highlight ? HIGH_BUTTON_TEXT_COLOR : BUTTON_TEXT_COLOR);
  b_text_top = two_line 
    ? b_top + 2 + (BUTTON_HEIGHT - 4 - 2 * text_size * CHAR_HEIGHT) / 2
    : b_top + 2 + (BUTTON_HEIGHT - 4 - text_size * CHAR_HEIGHT) / 2;

  tft.setCursor(b_left + 2 + (BUTTON_WIDTH - 4 - strlen(str_buffer1) * text_size * CHAR_WIDTH) / 2, b_text_top);
  tft.print(str_buffer1);
  if (two_line) {
    tft.setCursor(b_left + 2 + (BUTTON_WIDTH - 4 - strlen(str_buffer2) * text_size * CHAR_WIDTH) / 2, b_text_top + CHAR_HEIGHT);
    tft.print(str_buffer2);
  }
}


const byte INFO_RADIUS = BUTTON_RADIUS;

void display::draw_info_border(const byte button, const byte wide) {
  set_info_border(button, wide);
  tft.fillRect(info_left, info_top, info_wide, BUTTON_HEIGHT, BG_COLOR);
  tft.drawRoundRect(info_left+1, info_top+1, info_wide-2, BUTTON_HEIGHT-2, INFO_RADIUS, INFO_BORDER_COLOR);
  tft.fillRoundRect(info_left+2, info_top+2, info_wide-4, BUTTON_HEIGHT-4, INFO_RADIUS, INFO_BG_COLOR);
  info_bg_color = INFO_BG_COLOR;
}

void display::set_info_border(const byte button, const byte wide) {
  button_left_top(button, info_left, info_top);
  info_wide = wide * BUTTON_WIDTH;
  info_bg_color = BG_COLOR;
}

void display::update_keypad_entry(const long new_value) {
  update_info_number(new_value, KEYPAD_DIGITS, INFO_TEXT_SIZE, false);
}

void display::display_mini_label(MINI_LABEL ml, const bool show) {
  tft.setTextSize(MINI_LABEL_SIZE);
  tft.setCursor(MINI_LABEL_LEFT, MINI_LABEL_TOP + (byte) ml * CHAR_HEIGHT);
  tft.setTextColor(show ? MINI_LABEL_COLOR : BG_COLOR);
  strcpy_P(str_buffer1, (char *)pgm_read_word(&(mini_label[ml])));
  tft.print(str_buffer1);
}

void display::cross(const int16_t x, const int16_t y, const bool erase) {
  tft.drawLine(x, y - CROSS_SIZE, x, y + CROSS_SIZE, erase ? BG_COLOR : CROSS_FG);
  tft.drawLine(x - CROSS_SIZE, y, x + CROSS_SIZE, y, erase ? BG_COLOR : CROSS_FG);
}

void display::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  tft.fillRect(x, y, w, h, color);
}

void display::fillScreen(uint16_t color) {
  tft.fillScreen(color);
}

void display::center_text(const byte text_size, const uint16_t color) {
  tft.setCursor(
    (SCREEN_WIDTH - strlen(str_buffer1) * text_size * CHAR_WIDTH) / 2,
    (SCREEN_HEIGHT - (BANNER_SIZE + text_size) * CHAR_HEIGHT) / 2 + BANNER_SIZE * CHAR_HEIGHT);
  tft.setTextColor(color);
  tft.setTextSize(text_size);
  tft.print(str_buffer1);
}

void display::reset_persistants(void) {
  current_vfoa = current_vfob = current_info = __LONG_MAX__;
  color_vfoa = color_vfob = BG_COLOR;
}

void display::display_vfo(long new_value, long old_value, const uint16_t color, const uint16_t old_color,
                          const int16_t x, const int8_t y, const int8_t textsize) {
  bool erase_old = old_value != __LONG_MAX__;
  bool draw_new =  new_value != __LONG_MAX__;
  byte old_digit = 255, new_digit = 255;
  bool color_change = color != old_color;
 
  if (erase_old) old_value /= 10;
  if (draw_new) new_value /= 10;

  uint16_t _x = x + (6 * textsize - 1) * CHAR_WIDTH + 2 * textsize; 
  uint8_t  _y = y + CHAR_HEIGHT;
  byte _textsize = textsize - 1;

  for (byte i=0; i<7; i++) {
    if (erase_old) {
      old_digit = old_value % 10;
      old_value /= 10;
    }
    if (draw_new) {
      new_digit = new_value % 10;
      new_value /= 10;
    }

    // erase if needed
    if ((old_digit < 255) && (old_digit != new_digit))
      tft.drawChar(_x, _y, old_digit + '0', BG_COLOR, BG_COLOR, _textsize);  
  
    // draw if needed
    if ((new_digit < 255) && (color_change || (new_digit != old_digit)) && ((i < 6) || new_digit))
      tft.drawChar(_x, _y, new_digit + '0', color, color, _textsize); 

    // position for next character
    _x -= CHAR_WIDTH * _textsize; 

     if (i == 1) { 
       // font size increase Hz -> kHz
       _textsize++;
       _x -= CHAR_WIDTH;
       _y -= (CHAR_HEIGHT - 1);
     } else if (i == 4) {
       // decimal
       _x -= 2 * _textsize;
       if ((draw_new ^ erase_old) || color_change)
         tft.fillRect(_x + CHAR_WIDTH * _textsize, 
                      _y + (CHAR_HEIGHT - 2)  * _textsize, 
                      _textsize, _textsize,
                      draw_new ? color : BG_COLOR); 
    }
  }
}

const uint16_t VFO_COLORS[] = {
  INACTIVE_RX_VFO_COLOR,
  RX_VFO_COLOR,
  INACTIVE_TX_VFO_COLOR,
  TX_VFO_COLOR
};

void display::display_vfoa(const bool clear) {
  long vfo = clear ? __LONG_MAX__ : radio_obj.adjusted_frequency(false);
  uint16_t color = VFO_COLORS[radio_obj.vfo_state(false)];
  display_vfo(vfo, current_vfoa, color, color_vfoa, VFOA_LEFT, VFOA_TOP, VFOA_SIZE);
  current_vfoa = vfo;
  color_vfoa = color;
};

void display::display_vfob(const bool clear) {
  long vfo = clear ? __LONG_MAX__ : radio_obj.adjusted_frequency(true);
  uint16_t color = VFO_COLORS[radio_obj.vfo_state(true)];
  display_vfo(vfo, current_vfob, color, color_vfob, VFOB_LEFT, VFOB_TOP, VFOB_SIZE);
  current_vfob = vfo;
  color_vfob = color;
};

#ifdef USE_HB_DISP
void display::heartbeat(void) {
  const byte RECT_SIZE = 4;
  static bool on_off = false;
  on_off = !on_off;
  disp.fillRect(0, BANNER_SIZE * CHAR_HEIGHT,
                RECT_SIZE, RECT_SIZE,
                on_off ? HB_COLOR : BG_COLOR);
}
#endif

#ifdef USE_CAT_ACTIVITY
void display::cat_activity(void) {
  const byte RECT_SIZE = 4;
  static bool on_off = false;
  on_off = !on_off;
  disp.fillRect(0, BANNER_SIZE * CHAR_HEIGHT + RECT_SIZE,
                RECT_SIZE, RECT_SIZE,
                on_off ? CAT_HB_COLOR : BANNER_BG);
}
#endif

display disp;