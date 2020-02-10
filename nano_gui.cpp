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

void setupTouch(){
  int x1, y1, x2, y2, x3, y3, x4, y4;
  Point ts_point;
  
  displayClear(DISPLAY_BLACK);
  strncpy_P(b,(const char*)F("Click on the cross"),sizeof(b));
  displayText(b, 20,100, 200, 50, DISPLAY_WHITE, DISPLAY_BLACK, DISPLAY_BLACK);

  // TOP-LEFT
  displayHline(10,20,20,DISPLAY_WHITE);
  displayVline(20,10,20, DISPLAY_WHITE);

  while(!readTouch(&ts_point))
    delay(100);
  while(readTouch(&ts_point))
    delay(100);
   x1 = ts_point.x;
   y1 = ts_point.y; 

  //rubout the previous one
  displayHline(10,20,20,DISPLAY_BLACK);
  displayVline(20,10,20, DISPLAY_BLACK);

  delay(1000);
   
  //TOP RIGHT
  displayHline(290,20,20,DISPLAY_WHITE);
  displayVline(300,10,20, DISPLAY_WHITE);

  while(!readTouch(&ts_point))
    delay(100); 
  while(readTouch(&ts_point))
    delay(100);
   x2 = ts_point.x;
   y2 = ts_point.y; 

  displayHline(290,20,20,DISPLAY_BLACK);
  displayVline(300,10,20, DISPLAY_BLACK);

  delay(1000);

  //BOTTOM LEFT
  displayHline(10,220,20,DISPLAY_WHITE);
  displayVline(20,210,20, DISPLAY_WHITE);
  
  while(!readTouch(&ts_point))
    delay(100);
   x3 = ts_point.x;
   y3 = ts_point.y; 
     
  while(readTouch(&ts_point))
    delay(100);
  displayHline(10,220,20,DISPLAY_BLACK);
  displayVline(20,210,20, DISPLAY_BLACK);

  delay(1000);

  //BOTTOM RIGHT
  displayHline(290,220,20,DISPLAY_WHITE);
  displayVline(300,210,20, DISPLAY_WHITE);

  while(!readTouch(&ts_point))
    delay(100);
   x4 = ts_point.x;
   y4 = ts_point.y; 
     
  
  displayHline(290,220,20,DISPLAY_BLACK);
  displayVline(300,210,20, DISPLAY_BLACK);

  // we average two readings and divide them by half and store them as scaled integers 10 times their actual, fractional value
  //the x points are located at 20 and 300 on x axis, hence, the delta x is 280, we take 28 instead, to preserve fractional value,
  //there are two readings (x1,x2) and (x3, x4). Hence, we have to divide by 28 * 2 = 56 
  globalSettings.touchSlopeX = ((x4 - x3) + (x2 - x1))/56; 
  //the y points are located at 20 and 220 on the y axis, hence, the delta is 200. we take it as 20 instead, to preserve the fraction value 
  //there are two readings (y1, y2) and (y3, y4). Hence we have to divide by 20 * 2 = 40
  globalSettings.touchSlopeY = ((y3 - y1) + (y4 - y2))/40;
  
  //x1, y1 is at 20 pixels
  globalSettings.touchOffsetX = x1 + -((20 * globalSettings.touchSlopeX)/10);
  globalSettings.touchOffsetY = y1 + -((20 * globalSettings.touchSlopeY)/10);

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


