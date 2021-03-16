#include "keypad.h"
#include "loop_master.h"
#include "display.h"
#include "str_buffer.h"
#include "main_loop.h"
#include "texts.h"

const byte BUTTON_1 = 11;
const byte BUTTON_2 = 12;
const byte BUTTON_3 = 13;
const byte BUTTON_4 = 16;
const byte BUTTON_5 = 17;
const byte BUTTON_6 = 18;
const byte BUTTON_7 = 21;
const byte BUTTON_8 = 22;
const byte BUTTON_9 = 23;
const byte BUTTON_0 = 27;
const byte BUTTON_BS = 24;

const byte BUTTON_KHZ = 14;
const byte BUTTON_CANCEL = 19;

long keypad_loop::value; 
bool (*keypad_loop::return_value)(const long);

void keypad_loop::setup(void) {
  value = 0;
  disp.reset_persistants();
  disp.draw_info_border(7, 2);
  disp.update_keypad_entry(value);
}

void keypad_loop::set_parameters(const char *title, bool (*return_value)(const long)) {
  disp.banner_override = title;
  keypad_loop::return_value = return_value;
}

void keypad_loop::button_setup(void) {
  all_buttons    =
  draw_buttons   = BUTTON_BIT(BUTTON_1)
                 | BUTTON_BIT(BUTTON_2)
                 | BUTTON_BIT(BUTTON_3)
                 | BUTTON_BIT(BUTTON_4)
                 | BUTTON_BIT(BUTTON_5)
                 | BUTTON_BIT(BUTTON_6)
                 | BUTTON_BIT(BUTTON_7)
                 | BUTTON_BIT(BUTTON_8)
                 | BUTTON_BIT(BUTTON_9)
                 | BUTTON_BIT(BUTTON_0)
                 | BUTTON_BIT(BUTTON_BS)
                 | BUTTON_BIT(BUTTON_KHZ)
                 | BUTTON_BIT(BUTTON_CANCEL);

  textsize1_buttons = BUTTON_BIT(BUTTON_CANCEL);
}

void keypad_loop::button_text(const byte button) {
  str_buffer1[1] = '\0';
  switch (button) {
    case BUTTON_0: str_buffer1[0] = '0'; break;
    case BUTTON_1: str_buffer1[0] = '1'; break;
    case BUTTON_2: str_buffer1[0] = '2'; break;
    case BUTTON_3: str_buffer1[0] = '3'; break;
    case BUTTON_4: str_buffer1[0] = '4'; break;
    case BUTTON_5: str_buffer1[0] = '5'; break;
    case BUTTON_6: str_buffer1[0] = '6'; break;
    case BUTTON_7: str_buffer1[0] = '7'; break;
    case BUTTON_8: str_buffer1[0] = '8'; break;
    case BUTTON_9: str_buffer1[0] = '9'; break;

    case BUTTON_KHZ:    strcpy_P(str_buffer1, KHZ);    break;
    case BUTTON_CANCEL: strcpy_P(str_buffer1, CANCEL); break;
    case BUTTON_BS:     strcpy_P(str_buffer1, BS);     break;
  }
}

void keypad_loop::do_digit(const byte digit) {
  long new_value = value * 10 + digit;
  if (new_value <  100000l) {
    disp.update_keypad_entry(new_value);
    value = new_value;
  }
}

void keypad_loop::touch_release(const byte button) {
  switch(button) {
      case BUTTON_CANCEL:
        next = &loop_main;
        break;
      case BUTTON_KHZ:
        if (return_value(value)) {
          next = &loop_main;
        }  
        break;
      case BUTTON_BS:
        value /= 10;
        disp.update_keypad_entry(value);
        break;  
      case BUTTON_0: do_digit(0); break;
      case BUTTON_1: do_digit(1); break;
      case BUTTON_2: do_digit(2); break;
      case BUTTON_3: do_digit(3); break;
      case BUTTON_4: do_digit(4); break;
      case BUTTON_5: do_digit(5); break;
      case BUTTON_6: do_digit(6); break;
      case BUTTON_7: do_digit(7); break;
      case BUTTON_8: do_digit(8); break;
      case BUTTON_9: do_digit(9); break;
  }
}

keypad_loop loop_keypad;