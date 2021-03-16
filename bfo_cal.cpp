#include "display.h"
#include "radio.h"
#include "loop_master.h"
#include "parameter_limits.h"
#include "setup.h"
#include "setup_buttons.h"

const byte BFO_DIGITS  = 8;
const char BFO_STEP = 50;

void display::display_bfo_cal(const bool clear) {
  update_info_number(clear ? __LONG_MAX__ : radio_obj.usbCarrier, BFO_DIGITS, INFO_TEXT_SIZE, true);
}

void radio::toggle_bfo_cal(void) {
  in_bfo_cal = !in_bfo_cal;
  rotary_mode = in_bfo_cal ? RM_BFO : RM_VFO;
  loop_master::active->update_bfo_cal();
  if (in_cw_delay) loop_master::active->update_bfo_cal_display();
}

void radio::adj_bfo(const bool inc) {
  usbCarrier = adj_range(usbCarrier, inc, BFO_STEP, BFO_MIN, BFO_MAX);
  si5351bx_setfreq(0, usbCarrier);  
  setHWFrequencyRX();
  loop_master::active->update_bfo_cal_display();  
}

void setup_loop::update_bfo_cal(void) {
  highlight_clear_button(BUTTON_BFO, false);
  disp.display_bfo_cal(!radio_obj.in_bfo_cal);
}

void setup_loop::update_bfo_cal_display(void) {
  disp.display_bfo_cal(false);
}
