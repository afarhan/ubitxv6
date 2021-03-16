#include "loop_master.h"
#include <Arduino.h>
#include <EEPROM.h>

#include "rotary.h"
#include "heartbeat.h"
#include "display.h"
#include "str_buffer.h"
#include "touch.h"
#include "radio.h"
#include "cat.h"

const int16_t HOLD_MILLIS = 1000;
const int16_t TOUCH_MILLIS = 500;
const int16_t MIN_PRESS_Z = 500;

const byte BEEP_CT = 4;  // 125 Hz ticks to make beep sound

loop_master *loop_master::active;
loop_master *loop_master::next = NULL;
unsigned long loop_master::touch_millis = UINT32_MAX;

bool loop_master::need_beep;


void loop_master::do_loop(void) {
  active->loop();
}

void loop_master::init_screen(void) {
  touchscreen_buttons = true;
  init();
  disp.fillScreen(BG_COLOR);
  disp.banner();

  if (touchscreen_buttons) {
    twoline_buttons = textsize1_buttons = all_buttons = draw_buttons = 0;
    button_setup();
    textsize1_buttons |= twoline_buttons;

    for (uint8_t i=0; i < MAX_BUTTONS; i++) {
      if (BUTTON_BIT(i) & draw_buttons) {
          disp.draw_button_border(i);
          highlight_clear_button(i, false);
      }
    }
  }
}

void loop_master::make_active(void) {
    active = this;
    init_screen();
    setup();
}

void loop_master::highlight_clear_button(const byte button, bool highlight) {
  if (BUTTON_BIT(button) & draw_buttons) {
    str_buffer1[0] = str_buffer2[0] = '\0';
    button_text(button);
    disp.highlight_clear_button(button, highlight,
      (BUTTON_BIT(button) & textsize1_buttons) ? 1 : 2,
       BUTTON_BIT(button) & twoline_buttons);
  }
}

void loop_master::loop(void) {
  static byte highlight = 255;
  static bool moved_off = false;
  static bool in_calibration_touch = false;
  static byte beep_ct;
  long cur_millis = millis();


  if (cw_tx_change_state) {
    cw_tx_change_state = false;
    radio_obj.set_cw_oscillators();
  }

  if (rotary_state > R_INACTIVE) {
    radio_obj.rotary(rotary_state == R_INC);
    rotary_state = R_INACTIVE;
    return;
  }

  if (ptt_state > PTT_INACTIVE) {
    ptt_change(ptt_state == PTT_DOWN);
    ptt_state = PTT_INACTIVE;
  }

  if (switch_state > S_INACTIVE) {
    if (switch_state == S_LONG) hold_release(SWITCH);
    else touch_release(SWITCH);
    switch_state = S_INACTIVE;
    return;
  }

  if (need_sw_beep && (cur_millis - sw_down_millis > LONG_DOWN)) {
    radio_obj.sidetone(true);
    beep_ct = BEEP_CT;
    need_sw_beep = false;
  }

  if (tick_touchscreen) {
    tick_touchscreen = false;
    if (touchscreen_buttons) {
      // normal button operation
      bool have_touch = readTouch(p);
      scaleTouch(p);
      byte button = p.y * (SCREEN_WIDTH / BUTTON_WIDTH) + p.x;
      if ((BUTTON_BIT(button) | all_buttons) && touch(button)) {
        if (highlight < 255) {
          if (button != highlight) {
            if (!moved_off) {
              highlight_clear_button(highlight, false);
              moved_off = true;
            } 
          }
          bool long_touch = (cur_millis - touch_millis) > HOLD_MILLIS;
          if (have_touch) {
            if (long_touch && need_beep) {
              radio_obj.sidetone(true);
              beep_ct = BEEP_CT;
              need_beep = false;
            }
          } else {
            if (highlight == button) {
              highlight_clear_button(highlight, false);
              if (long_touch)
                hold_release(button);
              else
                touch_release(button);
            }              
            highlight = 255;
          }
        } else if (have_touch) {
          highlight = button;
          touch_millis = cur_millis;
          highlight_clear_button(highlight, true);
          moved_off = false;
          need_beep = true;
        }
      }
    } else {
      // calibration
      if (readTouch(p)) {
        in_calibration_touch = true;
      } else {
        if (in_calibration_touch) {
          in_calibration_touch = false;
          calibration_release();
        } 
      }
    }
  }

  if (tick_125Hz) {
    tick_125Hz = false;

    #ifdef USE_CAT
      CATcheck();
    #endif

    if (beep_ct) {
      if (!--beep_ct) radio_obj.sidetone(false);
    }

    hb_125Hz();
  }

  if (tick_1Hz) {
    tick_1Hz = false;
    #ifdef USE_HB_DISP
    disp.heartbeat();
    #endif
    hb_1Hz();
  }

  if (next) {
      loop_master *n = next;
      next = NULL;
      n->make_active();
  }
}