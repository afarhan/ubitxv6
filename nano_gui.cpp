#include <Arduino.h>
#include "colors.h"
#include "settings.h"
#include "ubitx.h"
#include "nano_gui.h"
#include "touch.h"

#include <SPI.h>
#include <avr/pgmspace.h>


/*****************
 * Begin TFT functions
 *****************/
#define ILI9341_CS_PIN TFT_CS
#define ILI9341_DC_PIN TFT_DC
#include "PDQ_MinLib/PDQ_ILI9341.h"
PDQ_ILI9341 tft;

#include "nano_font.h"

void displayInit(void){
  //Pulling this low 6 times should exit deep sleep mode
  pinMode(TFT_CS,OUTPUT);
  for(uint8_t i = 0; i < 6; ++i){
    digitalWrite(TFT_CS,HIGH);
    digitalWrite(TFT_CS,LOW);
  }
  digitalWrite(TFT_CS,HIGH);//Disable writing for now

  tft.begin();
  tft.setFont(ubitx_font);
  tft.setTextWrap(true);
  tft.setTextColor(DISPLAY_GREEN,DISPLAY_BLACK);
  tft.setTextSize(1);
  tft.setRotation(1);
}

void displayPixel(unsigned int x, unsigned int y, unsigned int c){
  tft.drawPixel(x,y,c);
}

void displayHline(unsigned int x, unsigned int y, unsigned int w, unsigned int c){
  tft.drawFastHLine(x,y,w,c);
}

void displayVline(unsigned int x, unsigned int y, unsigned int l, unsigned int c){
  tft.drawFastVLine(x,y,l,c);
}

void displayClear(unsigned int color){
  tft.fillRect(0,0,320,240,color);
}

void displayRect(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c){
  tft.drawRect(x,y,w,h,c);
}

void displayFillrect(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c){
  tft.fillRect(x,y,w,h,c);
}

void displayChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg) {
  tft.drawCharGFX(x,y,c,color,bg,1);
}

void displayRawText(const char *text, int x1, int y1, int w, int color, int background){
  tft.setTextColor(color,background);
  tft.setCursor(x1,y1);
  tft.setBound(x1,x1+w);
  tft.print(text);
}

void displayText(const char *const text, int x1, int y1, int w, int h, int color, int background, int border, TextJustification_e justification)
{
  displayFillrect(x1, y1, w ,h, background);
  displayRect(x1, y1, w ,h, border);

  int16_t x1_out;
  int16_t y1_out;
  uint16_t width_out;
  uint16_t height_out;
  tft.getTextBounds(text,x1,y1,&x1_out,&y1_out,&width_out,&height_out,w);
  if(TextJustification_e::Center == justification){
    x1 += (w - ( (int32_t)width_out + (x1_out-x1)))/2;
  }
  else if(TextJustification_e::Right == justification){
    x1 += w - ((int32_t)width_out + (x1_out-x1));
  }
  else{
    x1 += 2;//Give a little bit of padding from the border
  }
  y1 += (ubitx_font->yAdvance + h - ( (int32_t)height_out))/2;
  displayRawText(text,x1,y1,w,color,background);
}

void drawCross(int16_t x_center,int16_t y_center,uint16_t color)
{
  constexpr uint8_t HALF_SIZE = 10;
  displayHline(x_center-HALF_SIZE,y_center,2*HALF_SIZE,color);
  displayVline(x_center,y_center-HALF_SIZE,2*HALF_SIZE,color);
}

void setupTouch(){
  constexpr int16_t CROSS_CORNER_OFFSET = 20;
  constexpr Point CROSS_CORNER_POINTS [] = {
    {CROSS_CORNER_OFFSET,CROSS_CORNER_OFFSET},//Top left
    {PDQ_ILI9341::ILI9341_TFTHEIGHT-CROSS_CORNER_OFFSET,CROSS_CORNER_OFFSET},//Top right
    {CROSS_CORNER_OFFSET, PDQ_ILI9341::ILI9341_TFTWIDTH-CROSS_CORNER_OFFSET},//Bottom left
    {PDQ_ILI9341::ILI9341_TFTHEIGHT-CROSS_CORNER_OFFSET,PDQ_ILI9341::ILI9341_TFTWIDTH-CROSS_CORNER_OFFSET}//Bottom right
  };
  
  displayClear(DISPLAY_BLACK);
  strncpy_P(b,(const char*)F("Click on the cross"),sizeof(b));
  displayText(b, 20,100, 200, 50, DISPLAY_WHITE, DISPLAY_BLACK, DISPLAY_BLACK);

  Point cal_points[sizeof(CROSS_CORNER_POINTS)/sizeof(CROSS_CORNER_POINTS[0])];

  for(uint8_t i = 0; i < sizeof(CROSS_CORNER_POINTS)/sizeof(CROSS_CORNER_POINTS[0]); ++i){
    drawCross(CROSS_CORNER_POINTS[i].x,CROSS_CORNER_POINTS[i].y,DISPLAY_WHITE);
    while(!readTouch(&cal_points[i])){
      delay(100);
    }
    while(readTouch(&cal_points[i])){
      delay(100);
    }
    drawCross(CROSS_CORNER_POINTS[i].x,CROSS_CORNER_POINTS[i].y,DISPLAY_BLACK);
    delay(1000);//Ensure that nobody is pressing the screen before we do the next point
  }

  //We can get nicer scaling if we allow more resolution on the divisor
  constexpr int32_t SCALE_SENSITIVITY_MULTIPLIER = 10;

  const int16_t diff_x_top = cal_points[1].x - cal_points[0].x;
  const int16_t diff_x_bottom = cal_points[3].x - cal_points[2].x;
  constexpr int32_t diff_x_target = CROSS_CORNER_POINTS[1].x - CROSS_CORNER_POINTS[0].x;

  //Average the measured differences
  globalSettings.touchSlopeX = SCALE_SENSITIVITY_MULTIPLIER*(diff_x_top + diff_x_bottom) / (2*diff_x_target);

  const int16_t diff_y_left = cal_points[2].y - cal_points[0].y;
  const int16_t diff_y_right = cal_points[3].y - cal_points[1].y;
  constexpr int32_t diff_y_target = CROSS_CORNER_POINTS[2].y - CROSS_CORNER_POINTS[0].y;

  //Average the measured differences
  globalSettings.touchSlopeY = SCALE_SENSITIVITY_MULTIPLIER*(diff_y_left + diff_y_right) / (2*diff_y_target);

  globalSettings.touchOffsetX = cal_points[0].x - ((CROSS_CORNER_OFFSET * globalSettings.touchSlopeX)/SCALE_SENSITIVITY_MULTIPLIER);
  globalSettings.touchOffsetY = cal_points[0].y - ((CROSS_CORNER_OFFSET * globalSettings.touchSlopeY)/SCALE_SENSITIVITY_MULTIPLIER);

/*
  Serial.print(x1);Serial.print(':');Serial.println(y1);
  Serial.print(x2);Serial.print(':');Serial.println(y2);
  Serial.print(x3);Serial.print(':');Serial.println(y3);
  Serial.print(x4);Serial.print(':');Serial.println(y4);
  
  //for debugging
  Serial.print(globalSettings.touchSlopeX); Serial.print(' ');
  Serial.print(globalSettings.touchSlopeY); Serial.print(' ');
  Serial.print(globalSettings.touchOffsetX); Serial.print(' ');
  Serial.println(globalSettings.touchOffsetY); Serial.println(' ');
*/  
  SaveSettingsToEeprom();
  displayClear(DISPLAY_BLACK);
}


