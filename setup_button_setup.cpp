#include "setup.h"
#include "setup_buttons.h"
#include "loop_master.h"
#include "defines.h"

void setup_loop::button_setup(void) {
  all_buttons    =
  draw_buttons   = BUTTON_BIT(BUTTON_FREQ)
                 | BUTTON_BIT(BUTTON_BFO)
                 | BUTTON_BIT(BUTTON_DELAY)
                 | BUTTON_BIT(BUTTON_KEYER)
                 | BUTTON_BIT(BUTTON_TOUCH)
                 #ifdef USE_PDL_POL
                 | BUTTON_BIT(BUTTON_PADDLE)
                 #endif
                 | BUTTON_BIT(BUTTON_SAVE)
                 #ifdef USE_TUNE
                 | BUTTON_BIT(BUTTON_TUNE_PWR)
                 #endif
                 #ifdef USE_TX_DIS
                 | BUTTON_BIT(BUTTON_TX_DISABLE)
                 #endif
                 | BUTTON_BIT(BUTTON_CWP)
                 | BUTTON_BIT(BUTTON_CANCEL)
                 ;
                
  twoline_buttons = BUTTON_BIT(BUTTON_FREQ)
                  | BUTTON_BIT(BUTTON_DELAY)
                  #ifdef USE_PDL_POL
                  | BUTTON_BIT(BUTTON_PADDLE)
                  #endif
                  | BUTTON_BIT(BUTTON_BFO)
                  | BUTTON_BIT(BUTTON_TOUCH)
                  #ifdef USE_TUNE
                  | BUTTON_BIT(BUTTON_TUNE_PWR)
                  #endif
                  #ifdef USE_TX_DIS
                  | BUTTON_BIT(BUTTON_TX_DISABLE)
                  #endif
                  | BUTTON_BIT(BUTTON_CWP)
                  | BUTTON_BIT(BUTTON_KEYER)
                  ;

  textsize1_buttons = BUTTON_BIT(BUTTON_CANCEL)
                    | BUTTON_BIT(BUTTON_SAVE)
                    ;
}