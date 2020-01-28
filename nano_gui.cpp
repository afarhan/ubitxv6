#include <Arduino.h>
#include "settings.h"
#include "ubitx.h"
#include "nano_gui.h"

#include <SPI.h>
#include <avr/pgmspace.h>

struct Point ts_point;

/*
 * This formats the frequency given in f 
 */
void formatFreq(uint32_t freq, char* buff, uint16_t buff_size) {
  memset(buff, 0, buff_size);

  ultoa(freq, buff, DEC);
  uint8_t num_digits = strlen(buff);
  const uint8_t num_spacers = (num_digits-1) / 3;
  const uint8_t num_leading_digits_raw = num_digits % 3;
  const uint8_t num_leading_digits = (0 == num_leading_digits_raw) ? 3 : num_leading_digits_raw;

  if(0 == num_spacers){
    return;
  }

  buff += num_leading_digits;
  num_digits -= num_leading_digits;
  for(int i = num_digits-1; i >= 0; --i){
    buff[i + (i/3 + 1)] = buff[i];
  }
  for(unsigned int i = 0; i < num_spacers; ++i){
    memcpy_P(buff,F("."),1);
    buff += 4;
  }
}

void readTouchCalibration(){
  LoadSettingsFromEeprom();
  /* for debugging
  Serial.print(globalSettings.touchSlopeX); Serial.print(' ');
  Serial.print(globalSettings.touchSlopeY); Serial.print(' ');
  Serial.print(globalSettings.touchOffsetX); Serial.print(' ');
  Serial.println(globalSettings.touchOffsetY); Serial.println(' ');
  //*/
}

void writeTouchCalibration(){
  SaveSettingsToEeprom();
}

#define Z_THRESHOLD     400
#define Z_THRESHOLD_INT  75
#define MSEC_THRESHOLD  3

static uint32_t msraw=0x80000000;
static  int16_t xraw=0, yraw=0, zraw=0;
static uint8_t rotation = 1;

static int16_t touch_besttwoavg( int16_t x , int16_t y , int16_t z ) {
  int16_t da, db, dc;
  int16_t reta = 0;
  if ( x > y ) da = x - y; else da = y - x;
  if ( x > z ) db = x - z; else db = z - x;
  if ( z > y ) dc = z - y; else dc = y - z;

  if ( da <= db && da <= dc ) reta = (x + y) >> 1;
  else if ( db <= da && db <= dc ) reta = (x + z) >> 1;
  else reta = (y + z) >> 1;   //    else if ( dc <= da && dc <= db ) reta = (x + y) >> 1;

  return (reta);
}

static void touch_update(){
  int16_t data[6];

  uint32_t now = millis();
  if (now - msraw < MSEC_THRESHOLD) return;
  
  SPI.setClockDivider(SPI_CLOCK_DIV8);//2MHz
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0xB1 /* Z1 */);
  int16_t z1 = SPI.transfer16(0xC1 /* Z2 */) >> 3;
  int z = z1 + 4095;
  int16_t z2 = SPI.transfer16(0x91 /* X */) >> 3;
  z -= z2;
  if (z >= Z_THRESHOLD) {
    SPI.transfer16(0x91 /* X */);  // dummy X measure, 1st is always noisy
    data[0] = SPI.transfer16(0xD1 /* Y */) >> 3;
    data[1] = SPI.transfer16(0x91 /* X */) >> 3; // make 3 x-y measurements
    data[2] = SPI.transfer16(0xD1 /* Y */) >> 3;
    data[3] = SPI.transfer16(0x91 /* X */) >> 3;
  }
  else data[0] = data[1] = data[2] = data[3] = 0; // Compiler warns these values may be used unset on early exit.
  data[4] = SPI.transfer16(0xD0 /* Y */) >> 3;  // Last Y touch power down
  data[5] = SPI.transfer16(0) >> 3;
  digitalWrite(CS_PIN, HIGH);
  SPI.setClockDivider(SPI_CLOCK_DIV2);//Return to full speed for TFT

  if (z < 0) z = 0;
  if (z < Z_THRESHOLD) { // if ( !touched ) {
    // Serial.println();
    zraw = 0;
    return;
  }
  zraw = z;
  
  int16_t x = touch_besttwoavg( data[0], data[2], data[4] );
  int16_t y = touch_besttwoavg( data[1], data[3], data[5] );
  
  //Serial.printf("    %d,%d", x, y);
  //Serial.println();
  if (z >= Z_THRESHOLD) {
    msraw = now;  // good read completed, set wait
    switch (rotation) {
      case 0:
      xraw = 4095 - y;
      yraw = x;
      break;
      case 1:
      xraw = x;
      yraw = y;
      break;
      case 2:
      xraw = y;
      yraw = 4095 - x;
      break;
      default: // 3
      xraw = 4095 - x;
      yraw = 4095 - y;
    }
  }
}


bool readTouch(){
  touch_update();
  if (zraw >= Z_THRESHOLD) {
    ts_point.x = xraw;
    ts_point.y = yraw;
    //Serial.print(ts_point.x); Serial.print(",");Serial.println(ts_point.y);
    return true;
  }
  return false;
}

void scaleTouch(struct Point *p){
  p->x = ((long)(p->x - globalSettings.touchOffsetX) * 10L)/ (long)globalSettings.touchSlopeX;
  p->y = ((long)(p->y - globalSettings.touchOffsetY) * 10L)/ (long)globalSettings.touchSlopeY;

  //Serial.print(p->x); Serial.print(",");Serial.println(p->y);
}

/*****************
 * Begin TFT functions
 *****************/
#define ILI9341_CS_PIN TFT_CS
#define ILI9341_DC_PIN TFT_DC
#include "PDQ_MinLib/PDQ_ILI9341.h"
PDQ_ILI9341 tft;

#include "nano_font.h"


void xpt2046_Init(){
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
}

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

  xpt2046_Init();
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

void displayRawText(char *text, int x1, int y1, int w, int color, int background){
  tft.setTextColor(color,background);
  tft.setCursor(x1,y1);
  tft.setBound(x1,x1+w);
  tft.print(text);
}

void displayText(char *text, int x1, int y1, int w, int h, int color, int background, int border) {
  displayFillrect(x1, y1, w ,h, background);
  displayRect(x1, y1, w ,h, border);

  int16_t x1_out;
  int16_t y1_out;
  uint16_t width_out;
  uint16_t height_out;
  tft.getTextBounds(text,x1,y1,&x1_out,&y1_out,&width_out,&height_out,w);
  x1 += (w - ( (int32_t)width_out + (x1_out-x1)))/2;
  y1 += (ubitx_font->yAdvance + h - ( (int32_t)height_out))/2;
  displayRawText(text,x1,y1,w,color,background);
}

void setupTouch(){
  int x1, y1, x2, y2, x3, y3, x4, y4;
  
  displayClear(DISPLAY_BLACK);
  strncpy_P(b,(const char*)F("Click on the cross"),sizeof(b));
  displayText(b, 20,100, 200, 50, DISPLAY_WHITE, DISPLAY_BLACK, DISPLAY_BLACK);

  // TOP-LEFT
  displayHline(10,20,20,DISPLAY_WHITE);
  displayVline(20,10,20, DISPLAY_WHITE);

  while(!readTouch())
    delay(100);
  while(readTouch())
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

  while(!readTouch())
    delay(100); 
  while(readTouch())
    delay(100);
   x2 = ts_point.x;
   y2 = ts_point.y; 

  displayHline(290,20,20,DISPLAY_BLACK);
  displayVline(300,10,20, DISPLAY_BLACK);

  delay(1000);

  //BOTTOM LEFT
  displayHline(10,220,20,DISPLAY_WHITE);
  displayVline(20,210,20, DISPLAY_WHITE);
  
  while(!readTouch())
    delay(100);
   x3 = ts_point.x;
   y3 = ts_point.y; 
     
  while(readTouch())
    delay(100);
  displayHline(10,220,20,DISPLAY_BLACK);
  displayVline(20,210,20, DISPLAY_BLACK);

  delay(1000);

  //BOTTOM RIGHT
  displayHline(290,220,20,DISPLAY_WHITE);
  displayVline(300,210,20, DISPLAY_WHITE);

  while(!readTouch())
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
  writeTouchCalibration();
  displayClear(DISPLAY_BLACK);
}


