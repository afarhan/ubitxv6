#include "main_loop.h"
#include "main_buttons.h"
#include "radio.h"
#include "display.h"

void main_loop::ptt_change(const bool down) {
  radio_obj.set_ptt(down && !radio_obj.vfo_data[radio_obj.using_vfo_b].cw);
}