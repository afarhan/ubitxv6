#include "display.h"
#include "defines.h"
#include "radio.h"
#include "loop_master.h"
#include "main_loop.h"
#include "main_buttons.h"

#ifdef USE_IF_SHIFT

const long IF_SHIFT_RANGE = 9950;
const long IF_SHIFT_STEP = 50;

const byte IF_SHIFT_DIGITS  = 5;

void display::display_if_shift(const bool clear) {
  update_info_number(clear ? __LONG_MAX__ : radio_obj.if_shift_freq - radio_obj.usbCarrier, IF_SHIFT_DIGITS, INFO_TEXT_SIZE, true);
}

void radio::toggle_if_shift(const bool reset) {
  if (reset) {
    in_if_shift = false;
    if_shift_freq = usbCarrier;
    si5351bx_setfreq(0, if_shift_freq);  
    setHWFrequencyRX();
  } else
    in_if_shift = !in_if_shift;

  rotary_mode = in_if_shift ? RM_IF_SHIFT : RM_VFO;
  loop_master::active->update_if_shift();
  if (in_if_shift) loop_master::active->update_if_shift_display();
}

void radio::adj_if_shift(const bool inc) {
  if_shift_freq = adj_range(if_shift_freq - usbCarrier, inc, IF_SHIFT_STEP, -IF_SHIFT_RANGE, IF_SHIFT_RANGE) + usbCarrier;
  si5351bx_setfreq(0, if_shift_freq);  
  setHWFrequencyRX();
  loop_master::active->update_if_shift_display();
}

void main_loop::update_if_shift(void) {
  highlight_clear_button(BUTTON_IF_SHIFT, false);
  if (radio_obj.in_if_shift) {
    disp.display_vfoa(true);
    disp.display_if_shift(false);
  } else {
    disp.display_if_shift(true);
    disp.display_vfoa(false);
  }
}

void main_loop::update_if_shift_display(void) {
  disp.display_if_shift(false);
}
#endif
