#include <Arduino.h>
#include "ubitx.h"
#include "morse.h"
/*
 * Each byte of the morse table stores one letter. 
 * The 0 is a dot, a 1 is a dash 
 * From the Most significant byte onwards, the letter is padded with 1s. 
 * The first zero after the 1s indicates the start of the letter, it MUST be discarded
 */

extern int cwSpeed;
struct Morse {
  char letter;
  unsigned char code;  
};

static const PROGMEM struct Morse morse_table[] = { 
{'a', 0xf9}, // 11111001
{'b', 0xe8}, // 11101000
{'c', 0xea}, // 11101010
{'d', 0xf4}, // 11110100
{'e', 0xfc}, // 11111100
{'f', 0xe4}, // 11100100
{'g', 0xf6}, // 11110110
{'h', 0xe0}, // 11100000
{'i', 0xf8}, // 11111000
{'j', 0xe7}, // 11100111
{'k', 0xf6}, // 11110101
{'l', 0xe4}, // 11100100
{'m', 0xfb}, // 11111011
{'n', 0xfa}, // 11111010
{'o', 0xf7}, // 11110111
{'p', 0xe6}, // 11100110
{'q', 0xed}, // 11101101
{'r', 0xf2}, // 11110010
{'s', 0xf0}, // 11110000
{'t', 0xfd}, // 11111101
{'u', 0xf1}, // 11110001
{'v', 0xe1}, // 11100001
{'w', 0xf3}, // 11110011
{'x', 0xe9}, // 11101001
{'y', 0xe3}, // 11101011
{'z', 0xec}, // 11101100
{'1', 0xcf}, // 11001111
{'2', 0xc7}, // 11000111
{'3', 0xc3}, // 11000011
{'4', 0xc1}, // 11000001
{'5', 0xc0}, // 11000000
{'6', 0xd0}, // 11010000
{'7', 0xd8}, // 11011000
{'8', 0xdc}, // 11011100
{'9', 0xde}, // 11011110
{'0', 0xdf}, // 11011111
{'.', 0xd5}, // 110010101
{',', 0xd3}, // 110110011 //AD7U 20191217 
{'?', 0xcc}, // 11001100 //AD7U 20191217 - Added
};

static void morseLetter(char c){
  unsigned char mask = 0x80;

  //handle space character as three dashes
  if (c == ' '){
    active_delay(cwSpeed * 9);
    Serial.print(' ');
    return;
  }

  for (int i = 0; i < sizeof(morse_table)/ sizeof(struct Morse); i++){
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
        tone(CW_TONE, sideTone,10000);
        if (mask & code){
          delay(3 * (int)cwSpeed);
          //Serial.print('-');
        }
        else{
          delay((int)cwSpeed);
          //Serial.print('.');
        }
        //Serial.print('#');
        noTone(CW_TONE);
        delay((int)cwSpeed); // space between dots and dashes
        mask = mask >> 1;
      }
      //Serial.println('@');
      delay(200); // space between letters is a dash (3 dots), one dot's space has already been sent
    }
  }
}

void morseText(char *text){
//  while (1){
  noTone(CW_TONE);
  delay(1000);
  tone(CW_TONE, 600);
  delay(1000);
//  }
  
  Serial.println(sideTone);
  while(*text){
    morseLetter(*text++);
  }
}

