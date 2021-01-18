/*
 * This class deals with all of the radio settings,
 * so that other areas of the code doesn't have to
 * 
 * Example usage:
 *   LoadSettingsFromEeprom();
 *   Serial.println(globalSettings.vfoAFreq);
 *   globalSettings.vfoAFreq = 12345678;
 *   SaveSettingsToEeprom();
 *   Serial.println(globalSettings.vfoAFreq);
 */

#pragma once

#include <stdint.h>//uint8_t etc.

static const uint8_t NUM_QUICKLIST_SETTINGS = 4;

/*
 * Loads default values for all settings
 */
void LoadDefaultSettings();

/*
 * Loads all persistent settings from the EEPROM
 */
void LoadSettingsFromEeprom();

/*
 * Saves all persistent settings to the EEPROM
 * 
 * It's a little CPU-cycle-wasteful to save EVERYTHING
 * each time, but keeps things simple
 */
void SaveSettingsToEeprom();

/*
 * These are all of the settings
 * Note that not all settings are saved to the EEPROM
 */
enum Vfo_e : uint8_t
{
  VFO_A,
  VFO_B
};

enum VfoMode_e : uint8_t
{
  VFO_MODE_LSB = 2,
  VFO_MODE_USB = 3
};

struct VfoSettings_t
{
  uint32_t frequency;
  VfoMode_e mode;
};

enum TuningMode_e : uint8_t
{
  TUNE_SSB,
  TUNE_CW
};

/*
 * This is the definition of the settings/state variables
 */
struct SettingsRam
{
  uint32_t oscillatorCal;
  uint32_t usbCarrierFreq;

  Vfo_e activeVfo;
  VfoSettings_t vfoA;
  VfoSettings_t vfoB;

  VfoSettings_t quickList[4];

  uint32_t cwSideToneFreq;
  uint16_t cwDitDurationMs;
  uint16_t cwActiveTimeoutMs;

  int16_t touchSlopeX;
  int16_t touchSlopeY;
  int16_t touchOffsetX;
  int16_t touchOffsetY;

  bool ritOn;
  uint32_t ritFrequency;

  TuningMode_e tuningMode;

  bool splitOn;

  uint16_t reflected;
  uint16_t forward;
  bool isHighSWR;

  bool enableSWR;
  bool enablePTT;

  bool txActive;
  bool txCatActive;
  uint32_t cwExpirationTimeMs;
  bool morseMenuOn;
};

//This is the shared declaration
extern SettingsRam globalSettings;

//Some convenience functions
uint32_t GetActiveVfoFreq();
void SetActiveVfoFreq(uint32_t frequency);
VfoMode_e GetActiveVfoMode();
void SetActiveVfoMode(VfoMode_e mode);
