#include "touch.h"

#include "radio.h"
#include "defines.h"
#include "pins.h"
#include "display.h"
#include "point.h"

// #define DO_DEBUG

#include <SPI.h>

// #include "pin_definitions.h"
// #include "settings.h"

constexpr int16_t Z_THRESHOLD = 200;
constexpr uint8_t MSEC_THRESHOLD = 3;//Max sample rate is 125kHz, but we'll limit ourselves conservatively

constexpr uint8_t START_COMMAND = 1 << 7;
constexpr uint8_t CHANNEL_Y = 1 << 4;
constexpr uint8_t CHANNEL_Z1 = 3 << 4;
constexpr uint8_t CHANNEL_Z2 = 4 << 4;
constexpr uint8_t CHANNEL_X = 5 << 4;
constexpr uint8_t CHANNEL_TEMPERATURE = 7 << 4;
constexpr uint8_t USE_8_INSTEAD_OF_12_BIT = 1 << 3;
constexpr uint8_t USE_SINGLE_ENDED_MEASUREMENT = 1 << 2;
constexpr uint8_t POWER_OFF = 0 << 0;
constexpr uint8_t POWER_ADC = 1 << 0;
constexpr uint8_t POWER_REF = 2 << 0;
constexpr uint8_t POWER_ADC_REF = 3 << 0;

constexpr uint16_t MEASURE_X = START_COMMAND | POWER_ADC | CHANNEL_Y;//X and Y channel labelling flip due to screen orientation
constexpr uint16_t MEASURE_Y = START_COMMAND | POWER_ADC | CHANNEL_X;//X and Y channel labelling flip due to screen orientation
constexpr uint16_t MEASURE_Z1 = START_COMMAND | POWER_ADC | CHANNEL_Z1;
constexpr uint16_t MEASURE_Z2 = START_COMMAND | POWER_ADC | CHANNEL_Z2;

constexpr uint8_t RAW_READ_TO_12BIT_VALUE_SHIFT = 3;//16 bits read, zero-padded, but the MSB of the 16 is where the "BUSY" signal is asserted, so only need to shift by 3 instead of 4


uint32_t msraw=0x80000000;
int16_t xraw=0, yraw=0, zraw=0;
constexpr uint8_t rotation = 1;

SPISettings spiSettingsTouch(2000000, MSBFIRST, SPI_MODE0);

int16_t touch_besttwoavg( int16_t x , int16_t y , int16_t z ) {
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

uint16_t touchReadChannel(uint8_t channel_command){
                                                  // We assume that SPI.beginTransaction has already been called, and CS is LOW
  SPI.transfer(channel_command);                  // Throw away any bytes here
  const uint16_t tmpH = SPI.transfer(0) & 0x7F;   // Leading 0 (during "busy" signal), followed by bits 11-5
  const uint16_t tmpL = SPI.transfer(0);          // Bits 4-0, followed by 0s
  return tmpH << 5 | tmpL >> 3;
}

void touch_update(){
  uint32_t now = millis();
  if (now - msraw < MSEC_THRESHOLD){
    return;
  }
  
  SPI.beginTransaction(spiSettingsTouch);
  digitalWrite(PIN_TOUCH_CS, LOW);

  int16_t z1 = touchReadChannel(MEASURE_Z1); // ~0 when not pressed, increases with pressure
  int32_t z = z1;
  int16_t z2 = touchReadChannel(MEASURE_Z2); // ~4095 when not pressed, decreases with pressure
  z += (4095 - z2);

  #ifdef DO_DEBUG
  #ifdef USE_SERIAL
  if (z > Z_THRESHOLD) {
    Serial.print(F("z1:"));
    Serial.print(z1);
    Serial.print(F(" z2:"));
    Serial.print(z2);
    Serial.print(F(" z:"));
    Serial.println(z);
  }
  #endif
  #endif

  zraw = z;
  if (zraw < Z_THRESHOLD) { //Don't bother reading x/y if we're not being touched
    digitalWrite(PIN_TOUCH_CS, HIGH);
    SPI.endTransaction();
    return;
  }

  // make 3 x-y measurements
  int16_t data[6];
  data[0] = touchReadChannel(MEASURE_X);
  data[1] = touchReadChannel(MEASURE_Y);
  data[2] = touchReadChannel(MEASURE_X);
  data[3] = touchReadChannel(MEASURE_Y);
  data[4] = touchReadChannel(MEASURE_X);
  data[5] = touchReadChannel(MEASURE_Y & ~POWER_ADC_REF);//Turn off sensor

  digitalWrite(PIN_TOUCH_CS, HIGH);
  SPI.endTransaction();
  
  int16_t x = touch_besttwoavg( data[0], data[2], data[4] );
  int16_t y = touch_besttwoavg( data[1], data[3], data[5] );

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

void initTouch(){
  pinMode(PIN_TOUCH_CS, OUTPUT);
  digitalWrite(PIN_TOUCH_CS, HIGH);
}

bool readTouch(Point &touch_point_out){
  touch_update();
  touch_point_out.x = xraw;
  touch_point_out.y = yraw;
  touch_point_out.z = zraw;
  if (zraw >= Z_THRESHOLD) {
    #ifdef DO_DEBUG
    #ifdef USE_SERIAL
    Serial.print(F("readTouch found zraw of "));Serial.println(zraw);
    Serial.print(touch_point_out.x);
    Serial.print(",");
    Serial.print(touch_point_out.y);
    Serial.print(",");
    Serial.println(touch_point_out.z);
    #endif
    #endif
    return true;
  }
  return false;
}

void scaleTouch(Point &p) {
  p.x = ((long)(p.x - radio_obj.offset_x) * 10l) / (long) radio_obj.slope_x / BUTTON_WIDTH;
  p.y = ((long)(p.y - radio_obj.offset_y) * 10l) / (long) radio_obj.slope_y / BUTTON_HEIGHT;
  #ifdef DO_DEBUG
  #ifdef USE_SERIAL
  if (p.z > Z_THRESHOLD) {
    Serial.print(p.x); Serial.print(",");Serial.println(p.y);
  }
  #endif
  #endif
}
