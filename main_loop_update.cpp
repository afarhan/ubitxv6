#include "main_loop.h"
#include "main_buttons.h"
#include "radio.h"
#include "display.h"
#include "str_buffer.h"

void main_loop::update_vfo(const bool vfob) {
  if (vfob) disp.display_vfob(false);
  else disp.display_vfoa(false);
}

void main_loop::update_mode(void) {
  highlight_clear_button(BUTTON_MODE, false);
}

void main_loop::update_step(void) {
  highlight_clear_button(BUTTON_STEP, false);
}

#ifdef USE_ATTN
void main_loop::update_attn(void) {
  highlight_clear_button(BUTTON_ATTN, false);
}
#endif

void main_loop::update_split(void) {
  highlight_clear_button(BUTTON_SPLIT, false);
}

#ifdef USE_TUNE
void main_loop::update_tune(void) {
  highlight_clear_button(BUTTON_TUNE, false);
}
#endif

#ifdef USE_LOCK
void main_loop::update_lock(void) {
  highlight_clear_button(BUTTON_LOCK, false);
}
#endif

#ifdef USE_SPOT
void main_loop::update_spot(void) {
  highlight_clear_button(BUTTON_SPOT, false);
}
#endif

void main_loop::update_rit(void) {
  highlight_clear_button(BUTTON_RIT, false);
}


#ifdef USE_MEMORY
const byte MEMORY_BUTTONS[] PROGMEM = {BUTTON_M1, BUTTON_M2, BUTTON_M3, BUTTON_M4, BUTTON_M5};

void main_loop::update_memory(const byte which) {
  highlight_clear_button(pgm_read_byte_near(MEMORY_BUTTONS + which), false);
}
#endif

#ifdef USE_RF_SHIFT_STEP
void main_loop::update_rf_shift_step(void) {
  highlight_clear_button(BUTTON_RF_SHIFT, false);
}
#endif

