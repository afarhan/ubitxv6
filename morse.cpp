#include <Arduino.h>//tone

#include "encoder.h"
#include "morse.h"
#include "pin_definitions.h"
#include "settings.h"

struct Morse {
  char letter;
  unsigned char code;
};

/*
 * Each byte of the morse table stores one letter. 
 * The 0 is a dot, a 1 is a dash 
 * From the Most significant byte onwards, the letter is padded with 1s. 
 * The first zero after the 1s indicates the start of the letter, it MUST be discarded
 */
static const PROGMEM struct Morse morse_table[] = { 
{'a', 0b11111001},
{'b', 0b11101000},
{'c', 0b11101010},
{'d', 0b11110100},
{'e', 0b11111100},
{'f', 0b11100010},
{'g', 0b11110110},
{'h', 0b11100000},
{'i', 0b11111000},
{'j', 0b11100111},
{'k', 0b11110101},
{'l', 0b11100100},
{'m', 0b11111011},
{'n', 0b11111010},
{'o', 0b11110111},
{'p', 0b11100110},
{'q', 0b11101101},
{'r', 0b11110010},
{'s', 0b11110000},
{'t', 0b11111101},
{'u', 0b11110001},
{'v', 0b11100001},
{'w', 0b11110011},
{'x', 0b11101001},
{'y', 0b11101011},
{'z', 0b11101100},
{'1', 0b11001111},
{'2', 0b11000111},
{'3', 0b11000011},
{'4', 0b11000001},
{'5', 0b11000000},
{'6', 0b11010000},
{'7', 0b11011000},
{'8', 0b11011100},
{'9', 0b11011110},
{'0', 0b11011111},
{'.', 0b10010101},
{',', 0b10110011},
{'?', 0b10001100},
{ 2 , 0b11010101}, // ASCII 0x02 is Start of Text - <CT>
{ 4 , 0b10000101}, // ASCII 0x04 is End of Transmission - <CL> is too long for our encoding scheme in 8 bits, but <SK> fits
};

void morseLetter(char c, uint16_t dit_duration_ms){
  if(!globalSettings.morseMenuOn){
    return;
  }
  unsigned char mask = 0x80;

  //handle space character as three dashes
  if (c == ' '){
    delay(7 * dit_duration_ms);
    //Serial.print(' ');
    return;
  }

  for (unsigned int i = 0; i < sizeof(morse_table)/ sizeof(struct Morse); i++){
    struct Morse m;
    memcpy_P(&m, morse_table + i, sizeof(struct Morse));

    if (m.letter == tolower(c)){
      unsigned char code = m.code;
      //Serial.print(m.letter); Serial.println(' ');
  
      while(mask & code && mask > 1)
        mask = mask >> 1;
      //now we are at the first zero, skip and carry on
      mask = mask >> 1;
      while(mask){
        tone(PIN_CW_TONE, globalSettings.cwSideToneFreq,10000);
        if (mask & code){
          delay(3 * dit_duration_ms);
          //Serial.print('-');
        }
        else{
          delay(dit_duration_ms);
          //Serial.print('.');
        }
        //Serial.print('#');
        noTone(PIN_CW_TONE);
        delay(dit_duration_ms); // space between dots and dashes
        mask = mask >> 1;
      }
      //Serial.println('@');
      delay(2*dit_duration_ms); // space between letters is a dash (3 dots), one dot's space has already been sent
      break;//We've played the letter, so don't bother checking the rest of the list
    }
  }
}

static const uint8_t RELATIVE_OFFSET_HZ = 100;
void morseText(const char *text, uint16_t dit_duration_ms){
  int16_t total_counts = 0;
  morseBool(false);
  enc_read();//Don't count initial tone against total_counts
  while(*text && (abs(total_counts) < 10)){
    morseLetter(*text++, dit_duration_ms);
    total_counts += enc_read();
  }
}

void morseBool(bool val){
  if(!globalSettings.morseMenuOn){
    return;
  }
  tone(PIN_CW_TONE, globalSettings.cwSideToneFreq + (val ? RELATIVE_OFFSET_HZ : -RELATIVE_OFFSET_HZ));
  delay(3*globalSettings.cwDitDurationMs);
  noTone(PIN_CW_TONE);
  delay(3*globalSettings.cwDitDurationMs);
}
