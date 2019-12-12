#include <XPT2046_Touchscreen.h>
#include <SPI.h>
//#include "ubitx_font.h"
#include <avr/pgmspace.h>

#define TFT_CS    10        
#define TFT_RS    9      

// Color definitions
#define ILI9341_BLACK       0x0000  ///<   0,   0,   0
#define ILI9341_NAVY        0x000F  ///<   0,   0, 123
#define ILI9341_DARKGREEN   0x03E0  ///<   0, 125,   0
#define ILI9341_DARKCYAN    0x03EF  ///<   0, 125, 123
#define ILI9341_MAROON      0x7800  ///< 123,   0,   0
#define ILI9341_PURPLE      0x780F  ///< 123,   0, 123
#define ILI9341_OLIVE       0x7BE0  ///< 123, 125,   0
#define ILI9341_LIGHTGREY   0xC618  ///< 198, 195, 198
#define ILI9341_DARKGREY    0x7BEF  ///< 123, 125, 123
#define ILI9341_BLUE        0x001F  ///<   0,   0, 255
#define ILI9341_GREEN       0x07E0  ///<   0, 255,   0
#define ILI9341_CYAN        0x07FF  ///<   0, 255, 255
#define ILI9341_RED         0xF800  ///< 255,   0,   0
#define ILI9341_MAGENTA     0xF81F  ///< 255,   0, 255
#define ILI9341_YELLOW      0xFFE0  ///< 255, 255,   0
#define ILI9341_WHITE       0xFFFF  ///< 255, 255, 255
#define ILI9341_ORANGE      0xFD20  ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define ILI9341_PINK        0xFC18  ///< 255, 130, 198

#define TEXT_LINE_HEIGHT 18
#define TEXT_LINE_INDENT 5

#define BUTTON_PUSH
#define BUTTON_CHECK
#define BUTTON_SPINNER


GFXfont *gfxFont = NULL;
XPT2046_Touchscreen ts(CS_PIN);
TS_Point ts_point;

//filled from a test run of calibration routine
int slope_x=104, slope_y=137, offset_x=28, offset_y=29;

void readTouchCalibration(){
  EEPROM.get(SLOPE_X, slope_x);
  EEPROM.get(SLOPE_Y, slope_y);
  EEPROM.get(OFFSET_X, offset_x);
  EEPROM.get(OFFSET_Y, offset_y);  

  if (slope_x < 10 || 200 < slope_x){
    slope_x=104; 
    slope_y=137; 
    offset_x=28;
    offset_y=29;
  }  
}

void writeTouchCalibration(){
  EEPROM.put(SLOPE_X, slope_x);
  EEPROM.put(SLOPE_Y, slope_y);
  EEPROM.put(OFFSET_X, offset_x);
  EEPROM.put(OFFSET_Y, offset_y);    
}

boolean readTouch(){

  boolean istouched = ts.touched();  
  if (istouched) {
    ts_point = ts.getPoint();
    return true;
  }
  return false;
}

void scaleTouch(TS_Point *p){
  p->x = ((long)(p->x) * 10l)/(long)(slope_x) - offset_x;
  p->y = ((long)(p->y) * 10l)/(long)(slope_y) - offset_y;  
}


#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
 #define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
 #define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

inline GFXglyph * pgm_read_glyph_ptr(const GFXfont *gfxFont, uint8_t c)
{
#ifdef __AVR__
    return &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
#else
    // expression in __AVR__ section may generate "dereferencing type-punned pointer will break strict-aliasing rules" warning
    // In fact, on other platforms (such as STM32) there is no need to do this pointer magic as program memory may be read in a usual way
    // So expression may be simplified
    return gfxFont->glyph + c;
#endif //__AVR__
}

inline uint8_t * pgm_read_bitmap_ptr(const GFXfont *gfxFont)
{
#ifdef __AVR__
    return (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);
#else
    // expression in __AVR__ section generates "dereferencing type-punned pointer will break strict-aliasing rules" warning
    // In fact, on other platforms (such as STM32) there is no need to do this pointer magic as program memory may be read in a usual way
    // So expression may be simplified
    return gfxFont->bitmap;
#endif //__AVR__
}




inline static void utft_write(unsigned char d)
{
  SPI.transfer(d);
}

inline static void utftCmd(unsigned char VH)  
{   
  *(portOutputRegister(digitalPinToPort(TFT_RS))) &=  ~digitalPinToBitMask(TFT_RS);//LCD_RS=0;
  utft_write(VH);
}

inline static void utftData(unsigned char VH)
{
  *(portOutputRegister(digitalPinToPort(TFT_RS)))|=  digitalPinToBitMask(TFT_RS);//LCD_RS=1;
  utft_write(VH);
}
/*
void utftCmd_Data(unsigned char com,unsigned char dat)
{
  utftCmd(com);
  utftData(dat);
}
*/
static void utftAddress(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{
  utftCmd(0x2a);
  utftData(x1>>8);
  utftData(x1);
  utftData(x2>>8);
  utftData(x2);
        utftCmd(0x2b);
  utftData(y1>>8);
  utftData(y1);
  utftData(y2>>8);
  utftData(y2);
  utftCmd(0x2c);               
}

/*
inline void utftQuickfill(int16_t x1, int16_t y1, int16_t w, int16_t h, uint16_t color) {
    int16_t x2, y2;
    x2 = x1 + w;
    y2 = y1 + h;
    setAddrWindow(x, y, x2, y2);
    writeColor(color, (uint32_t)w * h);
}
*/

void utftPixel(unsigned int x, unsigned int y, unsigned int c)                   
{  
  unsigned int i,j;
  digitalWrite(TFT_CS,LOW);
  utftCmd(0x02c); //write_memory_start
  //digitalWrite(RS,HIGH);
  //l=l+x;
  utftAddress(x,y,x,y);
  //j=l*2;
  //for(i=1;i<=j;i++)
  //{
      utftData(c>>8);
      utftData(c);

    //utftData(c);
    //utftData(c);
  //}
  digitalWrite(TFT_CS,HIGH);   
}


void utftHline(unsigned int x, unsigned int y, unsigned int l, unsigned int c)                   
{  
  unsigned int i,j;
  digitalWrite(TFT_CS,LOW);
  utftCmd(0x02c); //write_memory_start
  //digitalWrite(RS,HIGH);
  l=l+x;
  utftAddress(x,y,l,y);
//  j=l*2;
  j = l;
  for(i=1;i<=j;i++)
  {
      utftData(c>>8);
      utftData(c);
  }
  digitalWrite(TFT_CS,HIGH);   
  checkCAT();
}

void displayClear(unsigned int j)                   
{  
  unsigned int i,m;
  digitalWrite(TFT_CS,LOW);
  utftAddress(0,0,320,240);
  for(i=0;i<320;i++)
    for(m=0;m<240;m++)
    {
      utftData(j>>8);
      utftData(j);
    }
  digitalWrite(TFT_CS,HIGH);   
}

void utftVline(unsigned int x, unsigned int y, unsigned int l, unsigned int c)                   
{ 
  unsigned int i,j;
  digitalWrite(TFT_CS,LOW);
  utftCmd(0x02c); //write_memory_start
  //digitalWrite(RS,HIGH);
  l=l+y;
  utftAddress(x,y,x,l);
  //j=l*2;
  j = l;
  for(i=1;i<=l;i++)
  { 
      utftData(c>>8);
      utftData(c);
  }
  digitalWrite(TFT_CS,HIGH);   
  checkCAT();
}

void utftRect(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c)
{
  utftHline(x  , y  , w, c);
  utftHline(x  , y+h, w, c);
  utftVline(x  , y  , h, c);
  utftVline(x+w, y  , h, c);
}

void utftFillrect(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c)
{
  unsigned int i;
  for(i=0;i<h;i++)
  {
    //utftHline(x  , y  , w, c);
    utftHline(x  , y+i, w, c);
  }
}

void displayInit(void)
{
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV4); // 4 MHz (half speed)
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);  

  gfxFont = &ubitx_font;
  pinMode(TFT_CS,OUTPUT);
  pinMode(TFT_RS,OUTPUT);


  digitalWrite(TFT_CS,LOW);  //CS
  utftCmd(0xCB);  
  utftData(0x39); 
  utftData(0x2C); 
  utftData(0x00); 
  utftData(0x34); 
  utftData(0x02); 

  utftCmd(0xCF);  
  utftData(0x00); 
  utftData(0XC1); 
  utftData(0X30); 

  utftCmd(0xE8);  
  utftData(0x85); 
  utftData(0x00); 
  utftData(0x78); 

  utftCmd(0xEA);  
  utftData(0x00); 
  utftData(0x00); 

  utftCmd(0xED);  
  utftData(0x64); 
  utftData(0x03); 
  utftData(0X12); 
  utftData(0X81); 

  utftCmd(0xF7);  
  utftData(0x20); 

  utftCmd(0xC0);    //Power control 
  utftData(0x23);   //VRH[5:0] 

  utftCmd(0xC1);    //Power control 
  utftData(0x10);   //SAP[2:0];BT[3:0] 

  utftCmd(0xC5);    //VCM control 
  utftData(0x3e);   //Contrast
  utftData(0x28); 

  utftCmd(0xC7);    //VCM control2 
  utftData(0x86);   //--

  utftCmd(0x36);    // Memory Access Control 
  utftData(0x28);   // Make this horizontal display   

  utftCmd(0x3A);    
  utftData(0x55); 

  utftCmd(0xB1);    
  utftData(0x00);  
  utftData(0x18); 

  utftCmd(0xB6);    // Display Function Control 
  utftData(0x08); 
  utftData(0x82);
  utftData(0x27);  

  utftCmd(0x11);    //Exit Sleep 
  delay(120); 
      
  utftCmd(0x29);    //Display on 
  utftCmd(0x2c); 
  digitalWrite(TFT_CS,HIGH);

  //now to init the touch screen controller
  ts.begin();
  ts.setRotation(1);

  readTouchCalibration();  
}


// Draw a character
/**************************************************************************/
/*!
   @brief   Draw a single character
    @param    x   Bottom left corner x coordinate
    @param    y   Bottom left corner y coordinate
    @param    c   The 8-bit font-indexed character (likely ascii)
    @param    color 16-bit 5-6-5 Color to draw chraracter with
    @param    bg 16-bit 5-6-5 Color to fill background with (if same as color, no background)
    @param    size_x  Font magnification level in X-axis, 1 is 'original' size
    @param    size_y  Font magnification level in Y-axis, 1 is 'original' size
*/
/**************************************************************************/
#define FAST_TEXT 1

void displayChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg) {
  c -= (uint8_t)pgm_read_byte(&gfxFont->first);
  GFXglyph *glyph  = pgm_read_glyph_ptr(gfxFont, c);
  uint8_t  *bitmap = pgm_read_bitmap_ptr(gfxFont);
  
  uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
  uint8_t  w  = pgm_read_byte(&glyph->width),
           h  = pgm_read_byte(&glyph->height);
  int8_t   xo = pgm_read_byte(&glyph->xOffset),
           yo = pgm_read_byte(&glyph->yOffset);
  uint8_t  xx, yy, bits = 0, bit = 0;
  int16_t  xo16 = 0, yo16 = 0;
      
  digitalWrite(TFT_CS,LOW);

#ifdef FAST_TEXT
    uint16_t hpc = 0; // Horizontal foreground pixel count
    for(yy=0; yy<h; yy++) {
      for(xx=0; xx<w; xx++) {
        if(bit == 0) {
          bits = pgm_read_byte(&bitmap[bo++]);
          bit  = 0x80;
        }
        if(bits & bit) hpc++;
        else {
          if (hpc) {
             utftHline(x+xo+xx-hpc, y+yo+yy, hpc, color);
            hpc=0;
          }
        }
        bit >>= 1;
      }
      // Draw pixels for this line as we are about to increment yy
      if (hpc) {
        utftHline(x+xo+xx-hpc, y+yo+yy, hpc, color);
        hpc=0;
      }
      checkCAT();      
    }
#else
    for(yy=0; yy<h; yy++) {
      for(xx=0; xx<w; xx++) {
        if(!(bit++ & 7)) {
          bits = pgm_read_byte(&bitmap[bo++]);
        }
        if(bits & 0x80) {
          utftPixel(x+xo+xx, y+yo+yy, color);
        }
        bits <<= 1;
      }
      checkCAT();
    }
#endif
}
/*
void displayChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg) {
  
  // Character is assumed previously filtered by write() to eliminate
  // newlines, returns, non-printable characters, etc.  Calling
  // drawChar() directly with 'bad' characters of font may cause mayhem!
  
  c -= (uint8_t)pgm_read_byte(&gfxFont->first);
  GFXglyph *glyph  = pgm_read_glyph_ptr(gfxFont, c);
  uint8_t  *bitmap = pgm_read_bitmap_ptr(gfxFont);
  
  uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
  uint8_t  w  = pgm_read_byte(&glyph->width),
           h  = pgm_read_byte(&glyph->height);
  int8_t   xo = pgm_read_byte(&glyph->xOffset),
           yo = pgm_read_byte(&glyph->yOffset);
  uint8_t  xx, yy, bits = 0, bit = 0;
  int16_t  xo16 = 0, yo16 = 0;
      
  digitalWrite(TFT_CS,LOW);

  for(yy=0; yy<h; yy++) {
      for(xx=0; xx<w; xx++) {
          if(!(bit++ & 7)) {
              bits = pgm_read_byte(&bitmap[bo++]);
          }
          if(bits & 0x80) {
                  utftPixel(x+xo+xx, y+yo+yy, color);
          }
//          else
//                  utftPixel(x+xo+xx, y+yo+yy, bg);
 
          bits <<= 1;
      }
      checkCAT();
  }
  digitalWrite(TFT_CS,HIGH);
}
*/

int displayTextExtent(char *text) {

  int ext = 0;
  while(*text){
    char c = *text++;
    uint8_t first = pgm_read_byte(&gfxFont->first);
    if((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last))) {
        GFXglyph *glyph  = pgm_read_glyph_ptr(gfxFont, c - first);
        ext += (uint8_t)pgm_read_byte(&glyph->xAdvance);    
    }
  }//end of the while loop of the characters to be printed
  return ext;
}

void displayRawText(char *text, int x1, int y1, int color, int background){
  while(*text){
    char c = *text++;
    
    uint8_t first = pgm_read_byte(&gfxFont->first);
    if((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last))) {
  
        GFXglyph *glyph  = pgm_read_glyph_ptr(gfxFont, c - first);
        uint8_t   w     = pgm_read_byte(&glyph->width),
                  h     = pgm_read_byte(&glyph->height);
        if((w > 0) && (h > 0)) { // Is there an associated bitmap?
            int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
            displayChar(x1, y1+TEXT_LINE_HEIGHT, c, color, background);
            checkCAT();
        }
        x1 += (uint8_t)pgm_read_byte(&glyph->xAdvance);    
    }
  }//end of the while loop of the characters to be printed
  
}

// The generic routine to display one line on the LCD 
void displayText(char *text, int x1, int y1, int w, int h, int color, int background, int border) {

  utftFillrect(x1, y1, w ,h, background);
  utftRect(x1, y1, w ,h, border);

  x1 += (w - displayTextExtent(text))/2;
  y1  += (h - TEXT_LINE_HEIGHT)/2;
  while(*text){
    char c = *text++;
    
    uint8_t first = pgm_read_byte(&gfxFont->first);
    if((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last))) {
  
        GFXglyph *glyph  = pgm_read_glyph_ptr(gfxFont, c - first);
        uint8_t   w     = pgm_read_byte(&glyph->width),
                  h     = pgm_read_byte(&glyph->height);
        if((w > 0) && (h > 0)) { // Is there an associated bitmap?
            int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
            displayChar(x1, y1+TEXT_LINE_HEIGHT, c, color, background);
            checkCAT();
        }
        x1 += (uint8_t)pgm_read_byte(&glyph->xAdvance);    
    }
  }//end of the while loop of the characters to be printed
}

void setupTouch(){
  int x1, y1, x2, y2, x3, y3, x4, y4;
  
  displayClear(ILI9341_BLACK);
  displayText("Click on the cross", 20,100, 200, 50, ILI9341_WHITE, ILI9341_BLACK, ILI9341_BLACK);

  // TOP-LEFT
  utftHline(10,20,20,ILI9341_WHITE);
  utftVline(20,10,20, ILI9341_WHITE);

  while(!readTouch())
    delay(100);
  while(readTouch())
    delay(100);
   x1 = ts_point.x;
   y1 = ts_point.y; 

  //rubout the previous one
  utftHline(10,20,20,ILI9341_BLACK);
  utftVline(20,10,20, ILI9341_BLACK);

  delay(1000);
   
  //TOP RIGHT
  utftHline(290,20,20,ILI9341_WHITE);
  utftVline(300,10,20, ILI9341_WHITE);

  while(!readTouch())
    delay(100); 
  while(readTouch())
    delay(100);
   x2 = ts_point.x;
   y2 = ts_point.y; 

  utftHline(290,20,20,ILI9341_BLACK);
  utftVline(300,10,20, ILI9341_BLACK);

  delay(1000);

  //BOTTOM LEFT
  utftHline(10,220,20,ILI9341_WHITE);
  utftVline(20,210,20, ILI9341_WHITE);
  
  while(!readTouch())
    delay(100);
   x3 = ts_point.x;
   y3 = ts_point.y; 
     
  while(readTouch())
    delay(100);
  utftHline(10,220,20,ILI9341_BLACK);
  utftVline(20,210,20, ILI9341_BLACK);

  delay(1000);

  //BOTTOM RIGHT
  utftHline(290,220,20,ILI9341_WHITE);
  utftVline(300,210,20, ILI9341_WHITE);

  while(!readTouch())
    delay(100);
   x4 = ts_point.x;
   y4 = ts_point.y; 
     
  
  utftHline(290,220,20,ILI9341_BLACK);
  utftVline(300,210,20, ILI9341_BLACK);

  // we average two readings and divide them by half and store them as scaled integers 10 times their actual value 
  slope_x = ((x4 - x3) + (x2 - x1))/60; // divide by 300 will give each span it's slope, we add two of them so division reduces to 150 and then by 10 to scale it up : 15
  slope_y = ((y3 - y1) + (y4 - y2))/44;
  //x1, y1 is at 10 pixels
  offset_x = x1 * 10 / slope_x - 10;
  offset_y = y1 * 10 / slope_y - 10;

/*  //for debugging
  Serial.print(slope_x); Serial.print(' ');
  Serial.print(slope_y); Serial.print(' ');
  Serial.print(offset_x); Serial.print(' ');
  Serial.println(offset_y); Serial.print(' ');
  */
  writeTouchCalibration();
  displayClear(ILI9341_BLACK);
}


