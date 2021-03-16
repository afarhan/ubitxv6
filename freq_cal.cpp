#include "display.h"
#include "radio.h"
#include "loop_master.h"
#include "parameter_limits.h"
#include "setup.h"
#include "setup_buttons.h"

const byte FREQ_DIGITS = 8;
const int MASTER_CAL_STEP = 175; // 875;

void display::display_freq_cal(const bool clear) {
  update_info_number(clear ? __LONG_MAX__ : radio_obj.calibration, FREQ_DIGITS, INFO_TEXT_SIZE, true);
}

void radio::toggle_freq_cal(void) {
  in_freq_cal = !in_freq_cal;
  rotary_mode = in_freq_cal ? RM_FREQ : RM_VFO;
  loop_master::active->update_freq_cal();
  if (in_cw_delay) loop_master::active->update_freq_cal_display();
}

void radio::adj_master_cal(const bool inc) {
  calibration = adj_range(calibration, inc, MASTER_CAL_STEP, MASTER_CAL_MIN, MASTER_CAL_MAX);
  si5351_set_calibration(calibration);
  loop_master::active->update_freq_cal_display();
}

void setup_loop::update_freq_cal(void) {
  highlight_clear_button(BUTTON_FREQ, false);
  disp.display_freq_cal(!radio_obj.in_freq_cal);
}

void setup_loop::update_freq_cal_display(void) {
  disp.display_freq_cal(false);
}
