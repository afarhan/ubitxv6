#include "setup.h"
#include "setup_buttons.h"
#include "radio.h"
#include "display.h"
#include "str_buffer.h"

#ifdef USE_PDL_POL
void setup_loop::update_paddle_polarity(void) {
  highlight_clear_button(BUTTON_PADDLE, false);
}
#endif

void setup_loop::update_keyer_mode(void) {
  highlight_clear_button(BUTTON_KEYER, false);
}

#ifdef USE_TX_DIS
void setup_loop::update_tx_disable(void) {
  highlight_clear_button(BUTTON_TX_DISABLE, false);
}
#endif
