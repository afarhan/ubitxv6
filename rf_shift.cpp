#include "display.h"
#include "radio.h"
#include "loop_master.h"
#include "parameter_limits.h"
#include "main_loop.h"
#include "main_buttons.h"
#include "defines.h"

#ifdef USE_RF_SHIFT

const int8_t RF_SHIFT_RANGE = 127;
const int8_t RF_SHIFT_STEP = 1;

const byte RF_SHIFT_DIGITS = 4;

void display::display_rf_shift(const bool clear) {
  update_info_number(clear ? __LONG_MAX__ : (radio_obj.rf_shift_freq - firstIF) / 1000L, RF_SHIFT_DIGITS, INFO_TEXT_SIZE, true);
}

void radio::toggle_rf_shift(const bool reset) {
  if (reset) {
    in_rf_shift = false;
    rf_shift_freq = firstIF;
    setHWFrequencyRX();
  } else
    in_rf_shift = !in_rf_shift;
  rotary_mode = in_rf_shift ? RM_RF_SHIFT : RM_VFO;
  loop_master::active->update_rf_shift();
  if (in_rf_shift) loop_master::active->update_rf_shift_display();
}

void radio::adj_rf_shift(const bool inc) {
  long edit_shift = (rf_shift_freq - firstIF) / 1000L;
  edit_shift = adj_range(edit_shift, inc, RF_SHIFT_STEP, -RF_SHIFT_RANGE, RF_SHIFT_RANGE);
  rf_shift_freq = firstIF + edit_shift * 1000L;
  setHWFrequencyRX();
  loop_master::active->update_rf_shift_display();
}

void main_loop::update_rf_shift(void) {
  highlight_clear_button(BUTTON_RF_SHIFT, false);
  if (radio_obj.in_rf_shift) {
    disp.display_vfoa(true);
    disp.display_rf_shift(false);
  } else {
    disp.display_rf_shift(true);
    disp.display_vfoa(false);
  }
}

void main_loop::update_rf_shift_display(void) {
  disp.display_rf_shift(false);
}

#endif