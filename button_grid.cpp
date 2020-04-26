#include "button_grid.h"

#include <Arduino.h>
#include <avr/pgmspace.h>

#include "color_theme.h"
#include "nano_gui.h"
#include "scratch_space.h"

void drawButton(const ButtonGrid_t *const button_grid_P,
                const Button *const button_P)
{
  uint16_t tc = COLOR_INACTIVE_TEXT;
  uint16_t bgc = COLOR_INACTIVE_BACKGROUND;
  const uint16_t bdc = COLOR_INACTIVE_BORDER;

  Button button;
  memcpy_P(&button,button_P,sizeof(button));
  switch(button.status())
  {
    case ButtonStatus_e::Stateless://Fallthrough intended
    case ButtonStatus_e::Inactive://Fallthrough intended
    default:
    {
      //Colors are initialized for this, so we're done
      break;
    }
    case ButtonStatus_e::Active:
    {
      tc = COLOR_ACTIVE_TEXT;
      bgc = COLOR_ACTIVE_BACKGROUND;
      break;
    }
  }

  if(nullptr != button.text){
    strncpy_P(b,button.text,sizeof(b));
  }
  else if(nullptr != button.text_override){
    button.text_override(b,sizeof(b));
  }
  else{
    //Something's messed up
    //Serial.println(F("No text for button!"));
    return;
  }

  ButtonGrid_t button_grid;
  memcpy_P(&button_grid,button_grid_P,sizeof(button_grid));

  uint8_t button_col = 0;
  uint8_t button_row = 0;
  bool found_it = false;
  for(button_col = 0; button_col < button_grid.num_button_cols && !found_it; ++button_col){
    for(button_row = 0; button_row < button_grid.num_button_rows && !found_it; ++button_row){
      Button* bp;
      memcpy_P(&bp,&(button_grid.buttons_P[button_row*button_grid.num_button_cols + button_col]),sizeof(bp));
      if(bp == button_P){
        found_it = true;
        break;
      }
    }
    if(found_it){
      break;
    }
  }

  Serial.print(button_col);Serial.print(",");Serial.print(button_row);Serial.print(" ");Serial.println(b);
  displayText(b,
              button_col * button_grid.button_pitch_x + button_grid.top_left_x,
              button_row * button_grid.button_pitch_y + button_grid.top_left_y,
              button_grid.button_width,
              button_grid.button_height,
              tc, bgc, bdc);
}

void drawButtonGrid(const ButtonGrid_t *const button_grid_P)
{
  ButtonGrid_t button_grid;
  memcpy_P(&button_grid,button_grid_P,sizeof(button_grid));

  Button* bp;
  for(uint8_t i = 0; i < button_grid.num_button_rows * button_grid.num_button_cols; ++i){
    memcpy_P(&bp, &(button_grid.buttons_P[i]), sizeof(bp));
    if(nullptr == bp){
      continue;
    }
    drawButton(button_grid_P,bp);
  }
}
