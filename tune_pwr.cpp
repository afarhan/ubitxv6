#include "display.h"
#include "radio.h"
#include "loop_master.h"
#include "parameter_limits.h"
#include "setup.h"
#include "setup_buttons.h"

#ifdef USE_TUNE
const byte TUNE_PWR_DIGITS = 3;
const int TUNE_PWR_STEP = 1;


void display::display_tune_pwr(const bool clear) {
  update_info_number(clear ? __LONG_MAX__ : radio_obj.tune_pwr * 10, TUNE_PWR_DIGITS, INFO_TEXT_SIZE, false);
}

void radio::toggle_tune_pwr(void) {
  in_tune_pwr = !in_tune_pwr;
  rotary_mode = in_tune_pwr ? RM_TUNE_PWR : RM_VFO;
  loop_master::active->update_tune_pwr();
  if (in_tune_pwr) loop_master::active->update_tune_pwr_display();
}

void radio::adj_tune_pwr(const bool inc) {
  tune_pwr = adj_range(tune_pwr, inc, TUNE_PWR_STEP, TUNE_PWR_MIN, TUNE_PWR_MAX);
  loop_master::active->update_tune_pwr_display();
}

void setup_loop::update_tune_pwr(void) {
  highlight_clear_button(BUTTON_TUNE_PWR, false);
  disp.display_tune_pwr(!radio_obj.in_tune_pwr);
}

void setup_loop::update_tune_pwr_display(void) {
  disp.display_tune_pwr(false);
}
#endif
