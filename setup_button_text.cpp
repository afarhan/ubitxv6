#include "setup.h"
#include "setup_buttons.h"
#include "str_buffer.h"
#include "radio.h"
#include "texts.h"
#include "display.h"

void setup_loop::button_text(const byte button) {
  switch(button) {
    case BUTTON_FREQ:
      disp.set_button_texts(FREQ1, CAL);
      break;
    case BUTTON_BFO:
      disp.set_button_texts(BFO, CAL);
      break;
    case BUTTON_DELAY:
      strcpy_P(str_buffer1, DELAY);
      disp.int_to_str_buffer(str_buffer2, radio_obj.cw_delaytime * CW_DELAY_MULTIPLE);
      break;
    case BUTTON_KEYER:
      strcpy_P(str_buffer1, KEYER);
      strcpy_P(str_buffer2, radio_obj.keyer_name());
      break;
    case BUTTON_TOUCH:
      disp.set_button_texts(TOUCH, CAL);
      break;
    #ifdef USE_PDL_POL
      case BUTTON_PADDLE:
        strcpy_P(str_buffer1, PADDLE);
        strcpy_P(str_buffer2, radio_obj.paddle_polarity());
        break;
    #endif
    case BUTTON_SAVE:
      strcpy_P(str_buffer1, SAVE);
      break;
    case BUTTON_CANCEL:
      strcpy_P(str_buffer1, CANCEL);
      break;
    #ifdef USE_TUNE
      case BUTTON_TUNE_PWR:
        strcpy_P(str_buffer1, TUNE_PWR);
        disp.int_to_str_buffer(str_buffer2, radio_obj.tune_pwr * 10);
        break;
    #endif
    #ifdef USE_TX_DIS
      case BUTTON_TX_DISABLE:
        disp.set_button_texts(TX_TEXT, radio_obj.tx_enable ? EN_TEXT : DIS_TEXT);
        break;
    #endif
    case BUTTON_CWP:
      strcpy_P(str_buffer1, CWP);
      disp.int_to_str_buffer(str_buffer2, radio_obj.sidetone_frequency);
      break;    
  }
}
