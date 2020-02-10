#ifndef _NANO_GUI_H_
#define _NANO_GUI_H_

enum TextJustification_e : uint8_t
{
  Left,
  Right,
  Center
};

void displayInit();
void displayClear(unsigned int color);
void displayPixel(unsigned int x, unsigned int y, unsigned int c);
void displayHline(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void displayVline(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void displayRect(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c);
void displayFillrect(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c);
void displayChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg);
void displayText(const char *const text, int x1, int y1, int w, int h, int color, int background, int border, TextJustification_e justification = TextJustification_e::Center);

#define TEXT_LINE_HEIGHT 18
#define TEXT_LINE_INDENT 5

#define BUTTON_PUSH
#define BUTTON_CHECK
#define BUTTON_SPINNER

#endif // _NANO_GUI_H_
