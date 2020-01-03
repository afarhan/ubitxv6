/**
 * These are the "magic" indices where these user changable settinngs are stored in the EEPROM
 */
static const uint8_t EEPROM_ADDR_MASTER_CAL = 0;//int32_t
//4 is currently unused, but may have been LSB_CAL on other versions
static const uint8_t EEPROM_ADDR_USB_CAL = 8;//uint32_t
//12 is currently unused, but may have been CW_SIDETONE on other versions?
static const uint8_t EEPROM_ADDR_VFO_A_FREQ  = 16;//uint32_t
static const uint8_t EEPROM_ADDR_VFO_B_FREQ  = 20;//uint32_t
static const uint8_t EEPROM_ADDR_CW_SIDETONE  = 24;//uint32_t
static const uint8_t EEPROM_ADDR_CW_DIT_TIME  = 28;//uint32_t
static const uint8_t EEPROM_ADDR_TOUCH_SLOPE_X  = 32;//int16_t
static const uint8_t EEPROM_ADDR_TOUCH_SLOPE_Y  = 36;//int16_t
static const uint8_t EEPROM_ADDR_TOUCH_OFFSET_X  = 40;//int16_t
static const uint8_t EEPROM_ADDR_TOUCH_OFFSET_Y  = 44;//int16_t
static const uint8_t EEPROM_ADDR_CW_DELAYTIME  = 48;
static const uint8_t EEPROM_ADDR_VFO_A_MODE = 256;
static const uint8_t EEPROM_ADDR_VFO_B_MODE = 257;
static const uint8_t EEPROM_ADDR_CW_KEY_TYPE = 358;

template<typename T>
bool LoadSane(T& dest,uint16_t addr, T min, T max)
{
    T read_value;
    EEPROM.get(addr,read_value);
    if((min <= read_value) && (read_value <= max)){
      dest = read_value;
      return true;
    }
    return false;
}

#include <EEPROM.h>
#include "settings.h"

//This is the non-extern version that actually gets built
SettingsRam globalSettings;

void LoadDefaultSettings()
{
  memset(&globalSettings,0x00,sizeof(globalSettings));

  globalSettings.oscillatorCal = 0;
  globalSettings.usbCarrierFreq = 11052000L;

  globalSettings.activeVfo = Vfo_e::VFO_A;
  globalSettings.vfoA.frequency = 7150000L;
  globalSettings.vfoA.mode = VFO_MODE_LSB;
  globalSettings.vfoB.frequency = 14150000L;
  globalSettings.vfoB.mode = VFO_MODE_USB;

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

  globalSettings.tuningMode = TuningMode_e::TUNE_VOICE;

  globalSettings.splitOn = false;

  globalSettings.txActive = false;
  globalSettings.txCatActive = false;
}

void LoadSettingsFromEeprom()
{
  LoadSane(globalSettings.usbCarrierFreq,EEPROM_ADDR_USB_CAL,11048000L,11060000L);
  LoadSane(globalSettings.vfoA.frequency,EEPROM_ADDR_VFO_A_FREQ,3500000L,30000000L);
  LoadSane(globalSettings.vfoB.frequency,EEPROM_ADDR_VFO_B_FREQ,3500000L,30000000L);
  LoadSane(globalSettings.cwSideToneFreq,EEPROM_ADDR_CW_SIDETONE,100,2000);
  LoadSane(globalSettings.cwDitDurationMs,EEPROM_ADDR_CW_DIT_TIME,10,1000);
  if(LoadSane(globalSettings.cwActiveTimeoutMs,EEPROM_ADDR_CW_DELAYTIME,10,100)){
    globalSettings.cwActiveTimeoutMs *= 10;//scale by 10 for legacy reasons
  }
  LoadSane(globalSettings.vfoA.mode,EEPROM_ADDR_VFO_A_MODE,VFO_MODE_LSB,VFO_MODE_USB);
  LoadSane(globalSettings.vfoB.mode,EEPROM_ADDR_VFO_B_MODE,VFO_MODE_LSB,VFO_MODE_USB);
  LoadSane(globalSettings.keyerMode,EEPROM_ADDR_CW_KEY_TYPE,KEYER_STRAIGHT,KEYER_IAMBIC_B);

  //No sanity check on these - cal your heart out
  EEPROM.get(EEPROM_ADDR_MASTER_CAL,globalSettings.masterCalibration);
  EEPROM.get(EEPROM_ADDR_TOUCH_SLOPE_X,globalSettings.touchSlopeX);
  EEPROM.get(EEPROM_ADDR_TOUCH_SLOPE_Y,globalSettings.touchSlopeY);
  EEPROM.get(EEPROM_ADDR_TOUCH_OFFSET_X,globalSettings.touchOffsetX);
  EEPROM.get(EEPROM_ADDR_TOUCH_OFFSET_Y,globalSettings.touchOffsetY);
}

void SaveSettingsToEeprom()
{
  EEPROM.put(EEPROM_ADDR_MASTER_CAL,globalSettings.masterCalibration);
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
}
