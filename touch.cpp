#include "touch.h"

#include <SPI.h>

#include "pin_definitions.h"
#include "settings.h"

constexpr int16_t Z_THRESHOLD = 400;
constexpr uint8_t MSEC_THRESHOLD = 3;

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

SPISettings spiSettingsTouch(2000000,MSBFIRST,SPI_MODE0);

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

void touch_update(){
  zraw = 0;//Default value to report

  uint32_t now = millis();
  if (now - msraw < MSEC_THRESHOLD){
    return;
  }
  
  SPI.beginTransaction(spiSettingsTouch);
  digitalWrite(PIN_TOUCH_CS, LOW);

  //NOTE: SPI transfers are synchronous, but require setup!
  //  What happens here is that we first send a command to measure
  //  an axis, then send our next measure command while receiving
  //  the measurement requested previously, which is why it looks
  //  like we're "off by one" between the variable names and the
  //  commands being issued
  SPI.transfer(MEASURE_Z1);
  int16_t z1 = SPI.transfer16(MEASURE_Z2) >> RAW_READ_TO_12BIT_VALUE_SHIFT;
  int32_t z = z1 + 4095;
  int16_t z2 = SPI.transfer16(MEASURE_X) >> RAW_READ_TO_12BIT_VALUE_SHIFT;
  z -= z2;
  Serial.print(F("z1:"));Serial.print(z1);Serial.print(F(" z2:"));Serial.print(z2);Serial.print(F(" z:"));Serial.println(z);
  if (z < Z_THRESHOLD) {
    SPI.transfer16(MEASURE_X & ~POWER_ADC_REF);//throw away the X measure so that we don't see it on the next check, and turn off the system
    SPI.transfer16(0);
    digitalWrite(PIN_TOUCH_CS, HIGH);
    SPI.endTransaction();
    return;
  }
  zraw = z;

  SPI.transfer16(MEASURE_X);  //throw away the first X measure, 1st is always noisy
  int16_t data[6];
  data[0] = SPI.transfer16(MEASURE_Y) >> RAW_READ_TO_12BIT_VALUE_SHIFT;
  data[1] = SPI.transfer16(MEASURE_X) >> RAW_READ_TO_12BIT_VALUE_SHIFT; // make 3 x-y measurements
  data[2] = SPI.transfer16(MEASURE_Y) >> RAW_READ_TO_12BIT_VALUE_SHIFT;
  data[3] = SPI.transfer16(MEASURE_X) >> RAW_READ_TO_12BIT_VALUE_SHIFT;
  data[4] = SPI.transfer16(MEASURE_Y) >> RAW_READ_TO_12BIT_VALUE_SHIFT;
  data[5] = SPI.transfer16(0) >> RAW_READ_TO_12BIT_VALUE_SHIFT;
  digitalWrite(PIN_TOUCH_CS, HIGH);
  SPI.endTransaction();
  
  int16_t x = touch_besttwoavg( data[0], data[2], data[4] );
  int16_t y = touch_besttwoavg( data[1], data[3], data[5] );
  
  //Serial.printf("    %d,%d", x, y);
  //Serial.println();
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

bool readTouch(Point *const touch_point_out){
  touch_update();
  Serial.print(F("readTouch found zraw of "));Serial.println(zraw);
  if (zraw >= Z_THRESHOLD) {
    touch_point_out->x = xraw;
    touch_point_out->y = yraw;
    //Serial.print(ts_point.x); Serial.print(",");Serial.println(ts_point.y);
    return true;
  }
  return false;
}

void scaleTouch(Point *const touch_point_in_out){
  touch_point_in_out->x = ((long)(touch_point_in_out->x - globalSettings.touchOffsetX) * 10L)/ (long)globalSettings.touchSlopeX;
  touch_point_in_out->y = ((long)(touch_point_in_out->y - globalSettings.touchOffsetY) * 10L)/ (long)globalSettings.touchSlopeY;

  //Serial.print(p->x); Serial.print(",");Serial.println(p->y);
}