#include "main_loop.h"
#include "main_buttons.h"

void main_loop::button_setup(void) {
  // buttons below are bits within a 32-bit long  (variables are defined in parent class)

  draw_buttons   =  BUTTON_BIT(BUTTON_RIT)
                 #if defined USE_HAM_BAND || defined USE_BC_BAND
                 | BUTTON_BIT(BUTTON_BAND)
                 #endif
                 | BUTTON_BIT(BUTTON_MODE)
                 | BUTTON_BIT(BUTTON_STEP)
                 | BUTTON_BIT(BUTTON_SPLIT)
                 | BUTTON_BIT(BUTTON_FINP)
                 | BUTTON_BIT(BUTTON_CWS)
                 #ifdef USE_TUNE
                 | BUTTON_BIT(BUTTON_TUNE)
                 #endif
                 #ifdef USE_LOCK
                 | BUTTON_BIT(BUTTON_LOCK)
                 #endif
                 #ifdef USE_IF_SHIFT
                 | BUTTON_BIT(BUTTON_IF_SHIFT)
                 #endif
                 #if defined USE_RF_SHIFT || defined USE_RF_SHIFT_STEP
                 | BUTTON_BIT(BUTTON_RF_SHIFT)
                 #endif
                 #ifdef USE_SPOT
                 | BUTTON_BIT(BUTTON_SPOT)
                 #endif
                 #ifdef USE_ATTN
                 | BUTTON_BIT(BUTTON_ATTN)
                 #endif
                 | BUTTON_BIT(BUTTON_SETUP)
                 #ifdef USE_MAN_SAVE
                 | BUTTON_BIT(BUTTON_SAVE)
                 #endif
                 #ifdef USE_MEMORY 
                 | BUTTON_BIT(BUTTON_M1)
                 | BUTTON_BIT(BUTTON_M2)
                 | BUTTON_BIT(BUTTON_M3)
                 | BUTTON_BIT(BUTTON_M4)
                 | BUTTON_BIT(BUTTON_M5)
                 #endif
                 ;
  
  all_buttons    = draw_buttons
                 | BUTTON_BIT(FREQ_DISP_A1)
                 | BUTTON_BIT(FREQ_DISP_A2)
                 | BUTTON_BIT(FREQ_DISP_B1)
                 | BUTTON_BIT(FREQ_DISP_B2)
                 ;
  
  // textsize1_buttons are buttons using font size 1, else font size 2
 
  // twoline_buttons are buttons with two lines of text, are automatically font size 1
  twoline_buttons = BUTTON_BIT(BUTTON_CWS)
                  #ifdef USE_MAN_SAVE
                  | BUTTON_BIT(BUTTON_SAVE)
                  #endif
                  #ifdef USE_IF_SHIFT
                  | BUTTON_BIT(BUTTON_IF_SHIFT)
                  #endif
                  #if defined USE_RF_SHIFT || defined USE_RF_SHIFT_STEP
                  | BUTTON_BIT(BUTTON_RF_SHIFT)
                  #endif
                  #ifdef USE_MEMORY
                  | BUTTON_BIT(BUTTON_M1)
                  | BUTTON_BIT(BUTTON_M2)
                  | BUTTON_BIT(BUTTON_M3)
                  | BUTTON_BIT(BUTTON_M4)
                  | BUTTON_BIT(BUTTON_M5)
                  #endif
                  ;
}