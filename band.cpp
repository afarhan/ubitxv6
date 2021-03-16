#include "band.h"

#include <Arduino.h>
#include "loop_master.h"
#include "str_buffer.h"
#include "display.h"
#include "texts.h"
#include "main_loop.h"

#if defined USE_HAM_BAND || defined USE_BC_BAND

const byte BUTTON_CANCEL  = 29;

#ifdef USE_HAM_BAND
const byte BUTTON_160M    =  5;
const byte BUTTON_80M     =  6;
const byte BUTTON_60M     =  7; 
const byte BUTTON_40M     =  8;
const byte BUTTON_30M     =  9;
const byte BUTTON_20M     = 10;
const byte BUTTON_17M     = 11;
const byte BUTTON_15M     = 12;
const byte BUTTON_12M     = 13;
const byte BUTTON_10M     = 14;

const byte BTN_OFFSET = 10;
#else
const byte BTN_OFFSET = 0;
#endif

#ifdef USE_BC_BAND
const byte BUTTON_MW      =  5 + BTN_OFFSET;
const byte BUTTON_90      =  6 + BTN_OFFSET;
const byte BUTTON_60      =  7 + BTN_OFFSET;
const byte BUTTON_49      =  8 + BTN_OFFSET;
const byte BUTTON_41      =  9 + BTN_OFFSET;
const byte BUTTON_31      = 10 + BTN_OFFSET;
const byte BUTTON_25      = 11 + BTN_OFFSET;
const byte BUTTON_22      = 12 + BTN_OFFSET;
const byte BUTTON_19      = 13 + BTN_OFFSET;
const byte BUTTON_16      = 14 + BTN_OFFSET;
#endif



void band_loop::button_setup(void) {
  all_buttons    =
  draw_buttons   = BUTTON_BIT(BUTTON_CANCEL)
                 #ifdef USE_HAM_BAND
                 | BUTTON_BIT(BUTTON_160M)
                 | BUTTON_BIT(BUTTON_80M)
                 | BUTTON_BIT(BUTTON_40M)
                 | BUTTON_BIT(BUTTON_60M)
                 | BUTTON_BIT(BUTTON_30M)
                 | BUTTON_BIT(BUTTON_20M)
                 | BUTTON_BIT(BUTTON_17M)
                 | BUTTON_BIT(BUTTON_15M)
                 | BUTTON_BIT(BUTTON_12M)
                 | BUTTON_BIT(BUTTON_10M)
                 #endif

                 #ifdef USE_BC_BAND
                 | BUTTON_BIT(BUTTON_MW)
                 | BUTTON_BIT(BUTTON_90)
                 | BUTTON_BIT(BUTTON_60)
                 | BUTTON_BIT(BUTTON_49)
                 | BUTTON_BIT(BUTTON_41)
                 | BUTTON_BIT(BUTTON_31)
                 | BUTTON_BIT(BUTTON_25)
                 | BUTTON_BIT(BUTTON_22)
                 | BUTTON_BIT(BUTTON_19)
                 | BUTTON_BIT(BUTTON_16)
                 #endif
                 ;

  textsize1_buttons = BUTTON_BIT(BUTTON_CANCEL);
}

void band_loop::set_band(const RADIO_BAND band) {
  radio_obj.set_band(band);
  next = &loop_main;
}

void band_loop::button_text(const byte button) {
  switch (button) {
    #ifdef USE_HAM_BAND
    case BUTTON_160M: disp.int_to_str_buffer(str_buffer1, 160); break;
    case BUTTON_80M:  disp.int_to_str_buffer(str_buffer1,  80); break;
    case BUTTON_60M:  disp.int_to_str_buffer(str_buffer1,  60); break;
    case BUTTON_40M:  disp.int_to_str_buffer(str_buffer1,  40); break;
    case BUTTON_30M:  disp.int_to_str_buffer(str_buffer1,  30); break;
    case BUTTON_20M:  disp.int_to_str_buffer(str_buffer1,  20); break;
    case BUTTON_17M:  disp.int_to_str_buffer(str_buffer1,  17); break;
    case BUTTON_15M:  disp.int_to_str_buffer(str_buffer1,  15); break;
    case BUTTON_12M:  disp.int_to_str_buffer(str_buffer1,  12); break;
    case BUTTON_10M:  disp.int_to_str_buffer(str_buffer1,  10); break;
    #endif

    #ifdef USE_BC_BAND
    case BUTTON_MW:   strcpy_P(str_buffer1, MW);                break;
    case BUTTON_90:   disp.int_to_str_buffer(str_buffer1,  90); break;
    case BUTTON_60:   disp.int_to_str_buffer(str_buffer1,  60); break;
    case BUTTON_49:   disp.int_to_str_buffer(str_buffer1,  49); break;
    case BUTTON_41:   disp.int_to_str_buffer(str_buffer1,  41); break;
    case BUTTON_31:   disp.int_to_str_buffer(str_buffer1,  31); break;
    case BUTTON_25:   disp.int_to_str_buffer(str_buffer1,  25); break;
    case BUTTON_22:   disp.int_to_str_buffer(str_buffer1,  22); break;
    case BUTTON_19:   disp.int_to_str_buffer(str_buffer1,  19); break;
    case BUTTON_16:   disp.int_to_str_buffer(str_buffer1,  16); break;
    #endif

    case BUTTON_CANCEL: strcpy_P(str_buffer1, CANCEL);          break;
  }
}

void band_loop::touch_release(const byte button) {
  switch (button) {
    #ifdef USE_HAM_BAND
    case BUTTON_160M: set_band(RB_160); break;
    case BUTTON_80M:  set_band(RB_80);  break;
    case BUTTON_40M:  set_band(RB_40);  break;
    case BUTTON_30M:  set_band(RB_30);  break;
    case BUTTON_20M:  set_band(RB_20);  break;
    case BUTTON_17M:  set_band(RB_17);  break;
    case BUTTON_15M:  set_band(RB_15);  break;
    case BUTTON_12M:  set_band(RB_12);  break;
    case BUTTON_10M:  set_band(RB_10);  break;

    case BUTTON_60M:
      radio_obj.set_60m();
      next = &loop_main;
      break;

    #endif
 
    #ifdef USE_BC_BAND
    case BUTTON_MW:   set_band(RB_MW);  break;
    case BUTTON_90:   set_band(RB_90);  break;
    case BUTTON_60:   set_band(RB_60);  break;
    case BUTTON_49:   set_band(RB_49);  break;
    case BUTTON_41:   set_band(RB_41);  break;
    case BUTTON_31:   set_band(RB_31);  break;
    case BUTTON_25:   set_band(RB_25);  break;
    case BUTTON_22:   set_band(RB_22);  break;
    case BUTTON_19:   set_band(RB_19);  break;
    case BUTTON_16:   set_band(RB_16);  break;
    #endif


    case BUTTON_CANCEL:
      next = &loop_main;
      break;
  }
}

#endif

#if defined USE_HAM_BAND || defined USE_BC_BAND
  band_loop loop_band;
#endif