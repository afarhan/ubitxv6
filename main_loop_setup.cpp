#include "main_loop.h"
#include "display.h"
#include "radio.h"

void main_loop::setup(void) {
  disp.reset_persistants();
  radio_obj.rotary_mode = RM_VFO; 
  disp.set_info_border(6, 2);

  update_vfo(false);
  update_vfo(true);
}