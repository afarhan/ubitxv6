#include <avr/pgmspace.h>

#include "bands.h"
#include "utils.h"

/*
 * These are the bands for USA. Your bands may vary
 */
struct Band_t {
  uint32_t min;
  uint32_t max;
  uint8_t band_meters;
  char name[3];//Two characters + null terminator. Fixed width so we don't need to build separate pointers
};

const char UNKNOWN_BAND_NAME [] PROGMEM = "??";

constexpr Band_t bands [] PROGMEM {
//  {       0UL,      255UL, 255, "U8"},//Utility conversion option
//  {       0UL,    65535UL, 254, "UF"},//Utility conversion option
//  {  530000UL,  1700000UL, 253, "AM"},//Broadcast AM, actually centers at 268, but uint8 can't do that
//  { 1800000UL,  2000000UL, 160, "A0"},//0xA0 is 160
  { 3500000UL,  4000000UL,  80, "80"},
//  { 5330500UL,  5403500UL,  60, "60"},
  { 7000000UL,  7300000UL,  40, "40"},
  {10100000UL, 10150000UL,  30, "30"},
  {14000000UL, 14350000UL,  20, "20"},
  {18068000UL, 18168000UL,  17, "17"},
  {21000000UL, 21450000UL,  15, "15"},
//  {24890000UL, 24990000UL,  12, "12"},
//  {26965000UL, 27405000UL,  11, "CB"},//Citizen's Band
  {28000000UL, 29700000UL,  10, "10"},
};
constexpr uint8_t NUM_BANDS = sizeof(bands)/sizeof(bands[0]);

int8_t findBandIndexFromBand(const uint8_t target_band)
{
  Band_t band;
  for(uint8_t i = 0; i < NUM_BANDS; ++i){
    memcpy_P(&band,&bands[i],sizeof(band));
    if(target_band == band.band_meters){
      return i;
    }
  }

  return -1;
}

int8_t findBandIndexFromFreq(uint32_t frequency)
{
  Band_t band;
  for(uint8_t i = 0; i < NUM_BANDS; ++i){
    memcpy_P(&band,&bands[i],sizeof(band));
    if(frequency <= band.max){
      if(band.min <= frequency){
        return i;
      }
      //No bands overlap, and they are ordered in strictly increasing frequency, so we need search no further
      return -1;
    }
  }

  return -1;
}

void getBandString(const unsigned long frequency,
                   char* band_string_out,
                   uint16_t max_string_length)
{
  int8_t band_index = findBandIndexFromFreq(frequency);
  if(-1 == band_index){
    strncpy_P(band_string_out,UNKNOWN_BAND_NAME,max_string_length);
  }
  else{
    Band_t band;
    memcpy_P(&band,&bands[band_index],sizeof(band));
    strncpy_P(band_string_out,band.name,max_string_length);
  }
}

uint32_t getFreqInBand(const uint32_t frequency,
                       const uint8_t target_band)
{
  int8_t target_band_index = findBandIndexFromBand(target_band);
  if(-1 == target_band_index){
    //Hard to target a band we don't know about...
    return frequency;
  }

  //See if we're currrently in a valid band
  int8_t current_band_index = findBandIndexFromFreq(frequency);

  if(-1 == current_band_index){
    //We're not in a known band - just go to the center of the target band
    Band_t band;
    memcpy_P(&band,&bands[target_band_index],sizeof(band));
    return band.min + ((band.max - band.min)/2/100)*100;//midpoint truncated to 100Hz resolution
  }
  else{
    //We're in a known band. Match the relative position in the target band.
    Band_t current_band;
    memcpy_P(&current_band,&bands[current_band_index],sizeof(current_band));
    Band_t target_band;
    memcpy_P(&target_band,&bands[target_band_index],sizeof(target_band));
    const uint32_t range_current = current_band.max - current_band.min;
    const uint32_t range_target = target_band.max - target_band.min;
    return (((frequency - current_band.min) * (uint64_t)range_target / range_current + target_band.min)/100)*100;//truncated 100Hz
  }
}

bool isFreqInBand(const uint32_t frequency,
                  const uint8_t check_band)
{
  int8_t band_index = findBandIndexFromBand(check_band);

  if(-1 == band_index){
    //Unknown band - can't be in it
    return false;
  }

  Band_t band;
  memcpy_P(&band,&bands[band_index],sizeof(band));
  if( (frequency <= band.max)
   && (band.min <= frequency)){
      return true;
  }

  return false;
}

