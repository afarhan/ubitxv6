#include <string.h>//memset
#include <stdint.h>
#include <EEPROM.h>
#include <Arduino.h>//only needed for debugging's Serial.print stuff
#include "nano_gui.h"//redrawVFOs() function
#include "settings.h"

/**
 * These are the "magic" indices where these user changable settinngs are stored in the EEPROM
 */
static const uint16_t EEPROM_ADDR_MASTER_CAL = 0;//int32_t
//4 is currently unused, but may have been LSB_CAL on other versions
static const uint16_t EEPROM_ADDR_USB_CAL = 8;//uint32_t
//12 is currently unused, but may have been CW_SIDETONE on other versions?
static const uint16_t EEPROM_ADDR_VFO_A_FREQ  = 16;//uint32_t
static const uint16_t EEPROM_ADDR_VFO_B_FREQ  = 20;//uint32_t
static const uint16_t EEPROM_ADDR_CW_SIDETONE  = 24;//uint32_t
static const uint16_t EEPROM_ADDR_CW_DIT_TIME  = 28;//uint32_t
static const uint16_t EEPROM_ADDR_TOUCH_SLOPE_X  = 32;//int16_t
static const uint16_t EEPROM_ADDR_TOUCH_SLOPE_Y  = 36;//int16_t
static const uint16_t EEPROM_ADDR_TOUCH_OFFSET_X  = 40;//int16_t
static const uint16_t EEPROM_ADDR_TOUCH_OFFSET_Y  = 44;//int16_t
static const uint16_t EEPROM_ADDR_MORSE_MENU = 46;//uint8_t
static const uint16_t EEPROM_ADDR_CW_DELAYTIME  = 48;//uint16_t
static const uint16_t EEPROM_ADDR_VFO_A_MODE = 256;//uint8_t
static const uint16_t EEPROM_ADDR_VFO_B_MODE = 257;//uint8_t
static const uint16_t EEPROM_ADDR_CW_KEY_TYPE = 358;//uint8_t
static const uint16_t EEPROM_ADDR_QUICKLIST_FREQ = 630;//uint32_t array
static const uint16_t EEPROM_ADDR_QUICKLIST_MODE = 710;//uint8_t array

template<typename T>
bool LoadSane(T& dest,uint16_t addr, T min, T max)
{
    T read_value;
    EEPROM.get(addr,read_value);
    if((min <= read_value) && (read_value <= max)){
      dest = read_value;
      //Serial.print(addr);
      //Serial.print(F(":"));
      //Serial.println(dest);
      return true;
    }
    //Serial.print(addr);
    //Serial.print(F(": Not valid: "));
    //Serial.print(read_value);
    //Serial.print(F(" Leaving value at "));
    //Serial.println(dest);
    return false;
}

//This is the non-extern version that actually gets built
SettingsRam globalSettings;

void LoadDefaultSettings()
{
  memset(&globalSettings,0x00,sizeof(globalSettings));

  globalSettings.oscillatorCal = 0L;
  globalSettings.usbCarrierFreq = 11052000UL;

  globalSettings.activeVfo = Vfo_e::VFO_A;
  globalSettings.vfoA.frequency = 7150000UL;
  globalSettings.vfoA.mode = VFO_MODE_LSB;
  globalSettings.vfoB.frequency = 14150000UL;
  globalSettings.vfoB.mode = VFO_MODE_USB;

  for(uint8_t i = 0; i < NUM_QUICKLIST_SETTINGS; ++i){
    globalSettings.quickList[i].frequency = i*1000000;
    globalSettings.quickList[i].mode = VfoMode_e::VFO_MODE_LSB;
  }

  globalSettings.keyerMode = KEYER_STRAIGHT;
  globalSettings.cwSideToneFreq = 800;
  globalSettings.cwDitDurationMs = 100;
  globalSettings.cwActiveTimeoutMs = 50;

  globalSettings.touchSlopeX = 104;
  globalSettings.touchSlopeY = 137;
  globalSettings.touchOffsetX = 28;
  globalSettings.touchOffsetY = 29;

  globalSettings.ritOn = false;
  globalSettings.ritFrequency = globalSettings.vfoA.frequency;

  globalSettings.tuningMode = TuningMode_e::TUNE_SSB;

  globalSettings.splitOn = false;

  globalSettings.txActive = false;
  globalSettings.txCatActive = false;
  globalSettings.cwExpirationTimeMs = 0;
  globalSettings.morseMenuOn = false;
}

void LoadSettingsFromEeprom()
{
  LoadSane(globalSettings.usbCarrierFreq,EEPROM_ADDR_USB_CAL,11048000UL,11060000UL);
  LoadSane(globalSettings.vfoA.frequency,EEPROM_ADDR_VFO_A_FREQ,500000UL+1,109000000UL-1);//Allow all freq supported by si5351 driver
  LoadSane(globalSettings.vfoB.frequency,EEPROM_ADDR_VFO_B_FREQ,500000UL+1,109000000UL-1);//Allow all freq supported by si5351 driver
  LoadSane(globalSettings.cwSideToneFreq,EEPROM_ADDR_CW_SIDETONE,100UL,2000UL);
  LoadSane(globalSettings.cwDitDurationMs,EEPROM_ADDR_CW_DIT_TIME,10U,1000U);
  if(LoadSane(globalSettings.cwActiveTimeoutMs,EEPROM_ADDR_CW_DELAYTIME,10U,100U)){
    globalSettings.cwActiveTimeoutMs *= 10;//scale by 10 for legacy reasons
  }
  LoadSane(globalSettings.vfoA.mode,EEPROM_ADDR_VFO_A_MODE,VFO_MODE_LSB,VFO_MODE_USB);
  LoadSane(globalSettings.vfoB.mode,EEPROM_ADDR_VFO_B_MODE,VFO_MODE_LSB,VFO_MODE_USB);
  LoadSane(globalSettings.keyerMode,EEPROM_ADDR_CW_KEY_TYPE,KEYER_STRAIGHT,KEYER_IAMBIC_B);

  uint8_t morse_on = 0;
  LoadSane(morse_on,EEPROM_ADDR_MORSE_MENU,(uint8_t)0,(uint8_t)1);
  globalSettings.morseMenuOn = morse_on;

  for(uint8_t i = 0; i < NUM_QUICKLIST_SETTINGS; ++i){
    LoadSane(globalSettings.quickList[i].frequency,EEPROM_ADDR_QUICKLIST_FREQ+(sizeof(uint32_t)*i),500000UL+1,109000000UL-1);//Allow all freq supported by si5351 driver
    LoadSane(globalSettings.quickList[i].mode,EEPROM_ADDR_QUICKLIST_MODE+(sizeof(uint8_t)*i),VFO_MODE_LSB,VFO_MODE_USB);
  }

  //No sanity check on these - cal your heart out
  EEPROM.get(EEPROM_ADDR_MASTER_CAL,globalSettings.oscillatorCal);
  EEPROM.get(EEPROM_ADDR_TOUCH_SLOPE_X,globalSettings.touchSlopeX);
  EEPROM.get(EEPROM_ADDR_TOUCH_SLOPE_Y,globalSettings.touchSlopeY);
  EEPROM.get(EEPROM_ADDR_TOUCH_OFFSET_X,globalSettings.touchOffsetX);
  EEPROM.get(EEPROM_ADDR_TOUCH_OFFSET_Y,globalSettings.touchOffsetY);
}

void SaveSettingsToEeprom()
{
  //Serial.println(F("Saving..."));
  EEPROM.put(EEPROM_ADDR_MASTER_CAL,globalSettings.oscillatorCal);
  EEPROM.put(EEPROM_ADDR_USB_CAL,globalSettings.usbCarrierFreq);
  EEPROM.put(EEPROM_ADDR_VFO_A_FREQ,globalSettings.vfoA.frequency);
  EEPROM.put(EEPROM_ADDR_VFO_B_FREQ,globalSettings.vfoB.frequency);
  EEPROM.put(EEPROM_ADDR_CW_SIDETONE,globalSettings.cwSideToneFreq);
  EEPROM.put(EEPROM_ADDR_CW_DIT_TIME,globalSettings.cwDitDurationMs);
  EEPROM.put(EEPROM_ADDR_TOUCH_SLOPE_X,globalSettings.touchSlopeX);
  EEPROM.put(EEPROM_ADDR_TOUCH_SLOPE_Y,globalSettings.touchSlopeY);
  EEPROM.put(EEPROM_ADDR_TOUCH_OFFSET_X,globalSettings.touchOffsetX);
  EEPROM.put(EEPROM_ADDR_TOUCH_OFFSET_Y,globalSettings.touchOffsetY);
  EEPROM.put(EEPROM_ADDR_CW_DELAYTIME,globalSettings.cwActiveTimeoutMs/10);//scale by 10 for legacy reasons
  EEPROM.put(EEPROM_ADDR_VFO_A_MODE,globalSettings.vfoA.mode);
  EEPROM.put(EEPROM_ADDR_VFO_B_MODE,globalSettings.vfoB.mode);
  EEPROM.put(EEPROM_ADDR_CW_KEY_TYPE,globalSettings.keyerMode);
  EEPROM.put(EEPROM_ADDR_MORSE_MENU,(uint8_t)globalSettings.morseMenuOn);

  for(uint8_t i = 0; i < NUM_QUICKLIST_SETTINGS; ++i){
    EEPROM.put(EEPROM_ADDR_QUICKLIST_FREQ+(sizeof(uint32_t)*i),globalSettings.quickList[i].frequency);
    EEPROM.put(EEPROM_ADDR_QUICKLIST_MODE+(sizeof(uint8_t)*i),globalSettings.quickList[i].mode);
  }
}

uint32_t GetActiveVfoFreq()
{
  if(VFO_A == globalSettings.activeVfo){
    return globalSettings.vfoA.frequency;
  }
  else{
    return globalSettings.vfoB.frequency;
  }
}

void SetActiveVfoFreq(uint32_t frequency)
{
  if(VFO_A == globalSettings.activeVfo)
  {
    globalSettings.vfoA.frequency = frequency;
  }
  else{
    globalSettings.vfoB.frequency = frequency;
  }
}

VfoMode_e GetActiveVfoMode()
{
  if(VFO_A == globalSettings.activeVfo){
    return globalSettings.vfoA.mode;
  }
  else{
    return globalSettings.vfoB.mode;
  }
}

void SetActiveVfoMode(VfoMode_e mode)
{
  if(VFO_A == globalSettings.activeVfo)
  {
    globalSettings.vfoA.mode = mode;
  }
  else{
    globalSettings.vfoB.mode = mode;
  }
  redrawVFOs();
}
