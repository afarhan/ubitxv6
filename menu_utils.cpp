#include "menu_utils.h"

#include <avr/pgmspace.h>

#include "button_grid.h"
#include "color_theme.h"
#include "morse.h"
#include "nano_gui.h"
#include "utils.h"

bool findPressedButton(const ButtonGrid_t *const button_grid_P,
                       Button *const button_out,
                       const Point touch_point)
{
  ButtonGrid_t button_grid;
  memcpy_P(&button_grid,button_grid_P,sizeof(button_grid));

  if((touch_point.x < button_grid.top_left_x)
   ||(touch_point.y < button_grid.top_left_y)
   ||(touch_point.x > (int16_t)button_grid.button_pitch_x * button_grid.num_button_cols)
   ||(touch_point.y > (int16_t)button_grid.button_pitch_y * button_grid.num_button_rows)){
     //touch point was outside the button grid
     return false;
   }

  uint8_t row = (touch_point.x - button_grid.top_left_x) / button_grid.button_pitch_x;
  uint8_t col = (touch_point.y - button_grid.top_left_y) / button_grid.button_pitch_y;
  int16_t x_max = (col * button_grid.button_pitch_x) + button_grid.button_width;
  int16_t y_max = (row * button_grid.button_pitch_y) + button_grid.button_height;
  if((touch_point.x > x_max)
   ||(touch_point.y > y_max)){
     //touch point was outside of the button
     return false;
  }

  Button* bp;
  memcpy_P(&bp,&(button_grid.buttons_P[row * button_grid.num_button_cols + col]),sizeof(bp));
  if(nullptr == bp){
    //no button there
    return false;
  }

  memcpy_P(button_out,bp,sizeof(*button_out));
  return true;
}

void indexToRowCol(const ButtonGrid_t *const button_grid,
                   const int8_t index,
                   uint8_t *const row_out,
                   uint8_t *const col_out)
{
  *row_out = index / button_grid->num_button_cols;
  *col_out = index % button_grid->num_button_cols;
}

void movePuck(const ButtonGrid_t *const button_grid,
              const int8_t old_index,
              const int8_t new_index)
{
  if(-1 != old_index){
    uint8_t row;
    uint8_t col;
    indexToRowCol(button_grid,old_index,&row,&col);
    displayRect(button_grid->top_left_x + button_grid->button_pitch_x * col,
                button_grid->top_left_y + button_grid->button_pitch_y * row,
                button_grid->button_width,
                button_grid->button_height,
                COLOR_INACTIVE_BORDER);
  }
  if(-1 != new_index){
    uint8_t row;
    uint8_t col;
    indexToRowCol(button_grid,new_index,&row,&col);
    displayRect(button_grid->top_left_x + button_grid->button_pitch_x * col,
                button_grid->top_left_y + button_grid->button_pitch_y * row,
                button_grid->button_width,
                button_grid->button_height,
                COLOR_ACTIVE_BORDER);
  }
}

void playButtonMorse(const Button *const button,
                     const MorsePlaybackType_e play_type)
{
  if(MorsePlaybackType_e::PlayText == play_type){
    morseText(button->text);
  }
  else{
    morseLetter(button->morse);
  }

  const ButtonStatus_e bs = button->status();
  if(ButtonStatus_e::Inactive == bs){
    morseBool(false);
  }
  else if(ButtonStatus_e::Active == bs){
    morseBool(true);
  }
}

void initSelector(int16_t *const raw_select_val_in_out,
                  const ButtonGrid_t *const button_grid_P,
                  const MorsePlaybackType_e play_type)
{
  ButtonGrid_t button_grid;
  memcpy_P(&button_grid,button_grid_P,sizeof(button_grid));

  *raw_select_val_in_out = 0;

  if(0 < button_grid.num_button_rows * button_grid.num_button_cols){
    Button button;
    Button* bp;
    memcpy_P(&bp,&(button_grid.buttons_P[0]),sizeof(bp));
    memcpy_P(&button,bp,sizeof(button));
    movePuck(&button_grid,-1,0);
    playButtonMorse(&button,play_type);
  }
}

void adjustSelector(int16_t *const raw_select_val_in_out,
                    int16_t knob,
                    const ButtonGrid_t *const button_grid_P,
                    const MorsePlaybackType_e play_type)
{
  ButtonGrid_t button_grid;
  memcpy_P(&button_grid,button_grid_P,sizeof(button_grid));

  const uint8_t num_buttons = button_grid.num_button_rows * button_grid.num_button_cols;
  const uint8_t prev_select = (*raw_select_val_in_out)/MENU_KNOB_COUNTS_PER_ITEM;
  *raw_select_val_in_out += knob;
  bool keep_trying = true;
  while(keep_trying){
    *raw_select_val_in_out = LIMIT(*raw_select_val_in_out,0,num_buttons*MENU_KNOB_COUNTS_PER_ITEM - 1);
    const uint8_t new_select = (*raw_select_val_in_out)/MENU_KNOB_COUNTS_PER_ITEM;
    if(prev_select != new_select){
      Button* bp;
      memcpy_P(&bp,&(button_grid.buttons_P[new_select]),sizeof(bp));
      if(nullptr == bp){
        if(new_select > prev_select){
          *raw_select_val_in_out += MENU_KNOB_COUNTS_PER_ITEM;
        }
        else{
          *raw_select_val_in_out -= MENU_KNOB_COUNTS_PER_ITEM;
        }
        if((*raw_select_val_in_out <= 0)
         ||(*raw_select_val_in_out >= num_buttons*MENU_KNOB_COUNTS_PER_ITEM - 1)){
           keep_trying = false;
         }
        continue;
      }

      Button new_button;
      memcpy_P(&new_button,bp,sizeof(new_button));

      movePuck(&button_grid,prev_select,new_select);
      playButtonMorse(&new_button,play_type);
    }
  }
}

void endSelector(const int16_t raw_select,
                 const ButtonGrid_t *const button_grid_P)
{
  ButtonGrid_t button_grid;
  memcpy_P(&button_grid,button_grid_P,sizeof(button_grid));

  uint8_t index = raw_select/MENU_KNOB_COUNTS_PER_ITEM;

  movePuck(&button_grid,index,-1);
}
