#include "button.h"
#include "nano_gui.h"
#include "color_theme.h"
#include "ubitx.h"//b

void drawButton(Button* button)
{
  uint16_t tc = COLOR_INACTIVE_TEXT;
  uint16_t bgc = COLOR_INACTIVE_BACKGROUND;
  const uint16_t bdc = COLOR_INACTIVE_BORDER;
  const char* text = nullptr;
  if(nullptr != button->text){
    text = button->text;
  }
  else if(nullptr != button->text_override){
    button->text_override(b,sizeof(b));
    text = b;
  }
  else{
    //Something's messed up
    //Serial.println(F("No text for button!"));
    return;
  }
  displayText(text, button->x, button->y, button->w, button->h, tc, bgc, bdc);
}
