#include "radio.h"
#include "display.h"
#include "main_loop.h"
#include "main_buttons.h"
#include "loop_master.h"
#include "parameter_limits.h"

const byte CWS_DIGITS = 2;

void display::display_cw_speed(const bool clear) {
  update_info_number(clear ? __LONG_MAX__ : radio_obj.cw_speed, CWS_DIGITS, INFO_TEXT_SIZE, false);
}

void radio::toggle_cw_speed(void) {
  in_cw_speed = !in_cw_speed;
  rotary_mode = in_cw_speed ? RM_CWS : RM_VFO;
  loop_master::active->update_cw_speed();
  if (in_cw_speed) loop_master::active->update_cw_speed_display();
}

void radio::adj_cw_speed(const bool inc) {
  cw_speed = adj_range(cw_speed, inc, 1, MIN_CWS, MAX_CWS);
  cw_millis = 1200 / cw_speed;
  loop_master::active->update_cw_speed_display();
}

void main_loop::update_cw_speed(void) {
  highlight_clear_button(BUTTON_CWS, false);
  if (radio_obj.in_cw_speed) {
    disp.display_vfoa(true);
    disp.display_cw_speed(false);
  } else {
    disp.display_cw_speed(true);
    disp.display_vfoa(false);
  }
}

void main_loop::update_cw_speed_display(void) {
  disp.display_cw_speed(false);
}
