#include "touchscreen_calibration.h"
#include "display.h"
#include "texts.h"
#include "setup.h"
#include "radio.h"
#include "str_buffer.h"

void ts_calibration_loop::init(void) {
  touchscreen_buttons = false;
  disp.banner_override = TOUCH_CAL;
}

void ts_calibration_loop::setup(void) {
  int x, y;
  ctr = 0;
  set_center(ctr, x, y);
  strcpy_P(str_buffer1, PUSH_ABORT);
  disp.center_text(TC_TEXT_SIZE, CROSS_FG);
  disp.cross(x, y, false);
}

const byte CROSS_INSET = 10;

/*
 * 0 (0b00) - left,  top
 * 1 (0b01) - right, top
 * 2 (0b10) - left,  bottom
 * 3 (0b11) - right, bottom
 */
void ts_calibration_loop::set_center(const byte ctr, int &x, int &y)
{  
  x = ctr & 1 ? 300 : 20;
  y = ctr > 1 ? 220 : 20;
}

void ts_calibration_loop::calibration_release(void) {
  int x, y;
  // erase previous cross
  set_center(ctr, x, y);
  disp.cross(x, y, true);
  if (ctr == 3) {
    // calculate calibration

    // we average two readings and divide them by half and store them as scaled integers 10 times their actual, fractional value
    // the x points are located at 20 and 300 on x axis, hence, the delta x is 280, we take 28 instead, to preserve fractional value,
    // there are two readings (x1,x2) and (x3, x4). Hence, we have to divide by 28 * 2 = 56 
    radio_obj.slope_x = ((p.x - x_act[2]) + (x_act[1] - x_act[0])) / 56; 
    // the y points are located at 20 and 220 on the y axis, hence, the delta is 200. we take it as 20 instead, to preserve the fraction value 
    // there are two readings (y1, y2) and (y3, y4). Hence we have to divide by 20 * 2 = 40
    radio_obj.slope_y = ((y_act[2] - y_act[0]) + (p.y - y_act[1])) / 40;

    // x1, y1 is at 20 pixels
    radio_obj.offset_x = x_act[0] + -((20 * radio_obj.slope_x)/10);
    radio_obj.offset_y = y_act[0] + -((20 * radio_obj.slope_y)/10);

    next = &loop_setup;
  } else {
    // save point info
    x_act[ctr] = p.x;
    y_act[ctr] = p.y;
    set_center(++ctr, x, y);
    disp.cross(x, y, false);
  }
}

void ts_calibration_loop::touch_release(const byte button) {
  if (button == SWITCH)
    next = &loop_setup;
}

ts_calibration_loop loop_ts_calibration;