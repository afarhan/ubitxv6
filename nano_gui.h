#ifndef _NANO_GUI_H_
#define _NANO_GUI_H_

/* UI functions */
struct Point {
  int x, y;
};
extern struct Point ts_point;

void displayInit();
void displayClear(unsigned int color);
void displayPixel(unsigned int x, unsigned int y, unsigned int c);
void displayHline(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void displayVline(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void displayRect(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c);
void displayFillrect(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c);
void displayChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg);
void displayRawText(char *text, int x1, int y1, int color, int background);
void displayText(char *text, int x1, int y1, int w, int h, int color, int background, int border);

void formatFreq(uint32_t freq, char* buff, uint16_t buff_size);

/* touch functions */
boolean readTouch();

void setupTouch();
void scaleTouch(struct Point *p);

// Color definitions
#define DISPLAY_BLACK       0x0000  ///<   0,   0,   0
#define DISPLAY_NAVY        0x000F  ///<   0,   0, 123
#define DISPLAY_DARKGREEN   0x03E0  ///<   0, 125,   0
#define DISPLAY_DARKCYAN    0x03EF  ///<   0, 125, 123
#define DISPLAY_MAROON      0x7800  ///< 123,   0,   0
#define DISPLAY_PURPLE      0x780F  ///< 123,   0, 123
#define DISPLAY_OLIVE       0x7BE0  ///< 123, 125,   0
#define DISPLAY_LIGHTGREY   0xC618  ///< 198, 195, 198
#define DISPLAY_DARKGREY    0x7BEF  ///< 123, 125, 123
#define DISPLAY_BLUE        0x001F  ///<   0,   0, 255
#define DISPLAY_GREEN       0x07E0  ///<   0, 255,   0
#define DISPLAY_CYAN        0x07FF  ///<   0, 255, 255
#define DISPLAY_RED         0xF800  ///< 255,   0,   0
#define DISPLAY_MAGENTA     0xF81F  ///< 255,   0, 255
#define DISPLAY_YELLOW      0xFFE0  ///< 255, 255,   0
#define DISPLAY_WHITE       0xFFFF  ///< 255, 255, 255
#define DISPLAY_ORANGE      0xFD20  ///< 255, 165,   0
#define DISPLAY_GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define DISPLAY_PINK        0xFC18  ///< 255, 130, 198

#define TEXT_LINE_HEIGHT 18
#define TEXT_LINE_INDENT 5

#define BUTTON_PUSH
#define BUTTON_CHECK
#define BUTTON_SPINNER

#endif // _NANO_GUI_H_
