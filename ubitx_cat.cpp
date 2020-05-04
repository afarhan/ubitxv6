#include <Arduino.h>
#include "nano_gui.h"
#include "scratch_space.h"
#include "settings.h"
#include "tuner.h"

/**
 * The CAT protocol is used by many radios to provide remote control to comptuers through
 * the serial port.
 *
 * This is very much a work in progress. Parts of this code have been liberally
 * borrowed from other GPLicensed works like hamlib.
 *
 * WARNING : This is an unstable version and it has worked with fldigi,
 * it gives time out error with WSJTX 1.8.0
 */

static const uint8_t FT817_MESSAGE_SIZE = 5;

//Data is ordered parameters 1-4, then command code last
enum CatDataIndex_e : uint8_t {
  P1 = 0,
  P2 = 1,
  P3 = 2,
  P4 = 3,
  CMD = 4
};

enum Ft817Command_e : uint8_t {
  //Listed in the order presented by FT-817ND_OM_ENG_E13771011.pdf
  OffBit = 0x80,
  LockOn = 0x00,
  LockOff = LockOn | OffBit,
  PttOn = 0x08,
  PttOff = PttOn | OffBit,
  SetFrequency = 0x01,//P1-P4 are BCD, 0x01 0x42 0x34 0x56 = 14.23456MHz
  OperatingMode = 0x07,//See OperatingMode_e for P1 decode
  ClarOn = 0x05,
  ClarOff = ClarOn | OffBit,
  ClarFrequency = 0xF5,//P1 is sign/direction (0x00 = +, - otherwise), P3-P4 are BCD, 0x12 0x34 = 12.34kHz
  VfoToggle = 0x81,
  SplitOn = 0x02,
  SplitOff = SplitOn | OffBit,
  RepeaterMode = 0x09,//See RepeaterMode_e for P1 decode
  RepeaterOffset = 0xF9,//P1-P4 are BCD
  CtcssDcsMode = 0x0A,//See CtcssDcsMode_e for P1 decode
  CtcssTone = 0x0B,//P1-P2 are BCD, 0x08 0x85 = 88.5MHz
  DcsTone = 0x0C,//P1-P2 are BCD, 0x00 0x23 = code 023
  ReadRxStatus = 0xE7,//Returns ReadRxStatus_t
  ReadTxStatus = 0xF7,//Returns ReadTxStatus_t
  ReadFreqAndMode = 0x03,//Returns current frequency (BCD, 4 bytes), then mode (OperatingMode_e)
  PowerOn = 0x0F,
  PowerOff = PowerOn | OffBit,
  //Unofficial commands
  ReadEeprom = 0xBB,
};

enum OperatingMode_e : uint8_t {
  LSB = 0x00,
  USB = 0x01,
  CW = 0x02,
  CWR = 0x03,//CW-reverse aka LSB CW
  AM = 0x04,
  FM = 0x08,
  DIG = 0x0A,
  PKT = 0x0C,
};

enum RepeaterMode_e : uint8_t {
  ShiftMinus = 0x09,
  ShiftPlus = 0x49,
  Simplex = 0x89,
};

enum CtcssDcsMode_e : uint8_t {
  DcsOn = 0x0A,
  CtcssOn = 0x2A,
  EncoderOn = 0x4A,
  Off = 0x8A,
};

struct ReadRxStatus_t {
  //Bitfields are not defined by the standard to be portable, which is unfortunate
  uint8_t Smeter : 4;//0x00 = S0, 0x09 = S9, etc.
  uint8_t Dummy : 1;
  uint8_t DiscriminatorCenteringOff : 1;
  uint8_t CodeUnmatched : 1;
  uint8_t SquelchSuppressionActive : 1;
};

struct ReadTxStatus_t {
  //Bitfields are not defined by the standard to be portable, which is unfortunate
  uint8_t PowerOutputMeter : 4;
  uint8_t Dummy : 1;
  uint8_t SplitOff : 1;
  uint8_t HighSwrDetected : 1;
  uint8_t PttOff : 1;
};

//Values based on http://www.ka7oei.com/ft817_memmap.html
//hamlib likes to read addresses 0x0064 and 0x007A, but including support for some others
enum Ft817Eeprom_e : uint16_t {
  VfoAndBankSelect = 0x0055,
  TuningModes = 0x0057,
  KeyerStatus = 0x0058,
  BandSelect = 0x0059,
  BeepVolume = 0x005C,
  CwPitch = 0x005E,
  CwWeight = 0x005F,
  CwDelay = 0x0060,
  SidetoneVolume = 0x0061,
  CwSpeed = 0x0062,
  VoxGain = 0x0063,
  CatBaudRate = 0x0064,
  SsbMicVolume = 0x0067,
  AmMicVolume = 0x0068,
  FmMicVolume = 0x0069,
  TxPower = 0x0079,
  AntennaSelectAndSplit = 0x007A,
  VfoAPhantomMode = 0x01E9,
};

//for broken protocol
static const uint16_t CAT_RECEIVE_TIMEOUT_MS = 500;

static const uint8_t ACK = 0;

uint8_t setHighNibble(uint8_t b, uint8_t v) {
  // Clear the high nibble
  b &= 0x0f;
  // Set the high nibble
  return b | ((v & 0x0f) << 4);
}

uint8_t setLowNibble(uint8_t b, uint8_t v) {
  // Clear the low nibble
  b &= 0xf0;
  // Set the low nibble
  return b | (v & 0x0f);
}

uint8_t getHighNibble(uint8_t b) {
  return (b >> 4) & 0x0f;
}

uint8_t getLowNibble(uint8_t b) {
  return b & 0x0f;
}

// Takes a number and produces the requested number of decimal digits, staring
// from the least significant digit.
//
void getDecimalDigits(unsigned long number, uint8_t* result,int digits) {
  for (int i = 0; i < digits; i++) {
    // "Mask off" (in a decimal sense) the LSD and return it
    result[i] = number % 10;
    // "Shift right" (in a decimal sense)
    number /= 10;
  }
}

// Takes a frequency and writes it into the CAT command buffer in BCD form.
//
void writeFreq(unsigned long freq, uint8_t* cmd) {
  // Convert the frequency to a set of decimal digits. We are taking 9 digits
  // so that we can get up to 999 MHz. But the protocol doesn't care about the
  // LSD (1's place), so we ignore that digit.
  uint8_t digits[9];
  getDecimalDigits(freq,digits,9);
  // Start from the LSB and get each nibble
  cmd[P4] = setLowNibble(cmd[P4],digits[1]);
  cmd[P4] = setHighNibble(cmd[P4],digits[2]);
  cmd[P3] = setLowNibble(cmd[P3],digits[3]);
  cmd[P3] = setHighNibble(cmd[P3],digits[4]);
  cmd[P2] = setLowNibble(cmd[P2],digits[5]);
  cmd[P2] = setHighNibble(cmd[P2],digits[6]);
  cmd[P1] = setLowNibble(cmd[P1],digits[7]);
  cmd[P1] = setHighNibble(cmd[P1],digits[8]);
}

// This function takes a frquency that is encoded using 4 uint8_ts of BCD
// representation and turns it into an long measured in Hz.
//
// [12][34][56][78] = 123.45678? Mhz
//
uint32_t readFreq(uint8_t* cmd) {
    // Pull off each of the digits
    unsigned long ret = 0;
    for(uint8_t i = 0; i < 4; ++i){
      const uint8_t d1 = getHighNibble(cmd[i]);
      const uint8_t d0 = getLowNibble(cmd[i]);
      ret *= 100;
      ret += 10*d1 + d0;
    }

    return ret*10;
}

void catGetEeprom(const uint16_t read_address, uint8_t* response)
{
  switch (read_address)
  {
    case Ft817Eeprom_e::VfoAndBankSelect:
      //0 : VFO A/B  0 = VFO-A, 1 = VFO-B
      //1 : MTQMB Select  0 = (Not MTQMB), 1 = MTQMB ("Memory Tune Quick Memory Bank")
      //2 : QMB Select  0 = (Not QMB), 1 = QMB  ("Quick Memory Bank")
      //3 :
      //4 : Home Select  0 = (Not HOME), 1 = HOME memory
      //5 : Memory/MTUNE select  0 = Memory, 1 = MTUNE
      //6 :
      //7 : MEM/VFO Select  0 = Memory, 1 = VFO (A or B - see bit 0)
      *response = 0x80 //always report VFO mode
                | ((VFO_B == globalSettings.activeVfo) ? 0x01 : 0x00);
      break;
    case Ft817Eeprom_e::CwPitch:
      //3-0 : CW Pitch (300-1000 Hz) (#20)  From 0 to E (HEX) with 0 = 300 Hz and each step representing 50 Hz
      //5-4 :  Lock Mode (#32) 00 = Dial, 01 = Freq, 10 = Panel
      //7-6 :  Op Filter (#38) 00 = Off, 01 = SSB, 10 = CW
      *response = (globalSettings.cwSideToneFreq - 300)/50;
      break;
    case Ft817Eeprom_e::SidetoneVolume:
      //Sidetone (Volume) (#44) 0-100
      *response = globalSettings.cwSideToneFreq / 100;
      break;
    case Ft817Eeprom_e::CwDelay:
      //CW Delay (10-2500 ms) (#17)  From 1 to 250 (decimal) with each step representing 10 ms
      *response = globalSettings.cwActiveTimeoutMs / 10;
      break;
    case Ft817Eeprom_e::CwSpeed:
      //5-0  CW Speed (4-60 WPM) (#21) From 0 to 38 (HEX) with 0 = 4 WPM and 38 = 60 WPM (1 WPM steps)
      //7-6  Batt-Chg (6/8/10 Hours (#11)  00 = 6 Hours, 01 = 8 Hours, 10 = 10 Hours
      *response = (1200 / globalSettings.cwDitDurationMs) - 4;
      break;
    case Ft817Eeprom_e::CatBaudRate:
      //4-0 : VOX Delay (#50) 0 = 100 Ms with each step representing 100 Ms. 24 = 2500 Ms
      //5 : Emergency (#28) 0 = Off, 1 = On
      //7-6 : CAT Rate (4800, 9600, 38400) (#14) 00 = 4800, 01 = 9600, 10 = 38400 Baud
      *response = 0xA5;
      break;
    case Ft817Eeprom_e::VfoAPhantomMode:
      //2-0 : 000 = LSB, 001 = USB, 010 = CW, 011 = CWR, 100 = AM, 101 = FM, 110 = DIG, 111 = PKT
      //7-3 : ?
      if (VfoMode_e::VFO_MODE_USB == GetActiveVfoMode()){
        *response = OperatingMode_e::USB;
      }
      else{
        *response = OperatingMode_e::LSB;
      }
      break;
    case Ft817Eeprom_e::AntennaSelectAndSplit:
      //0 : HF Antenna Select 0 = Front, 1 = Rear
      //1 : 6 M Antenna Select  0 = Front, 1 = Rear
      //2 : FM BCB Antenna Select 0 = Front, 1 = Rear
      //3 : Air Antenna Select  0 = Front, 1 = Rear
      //4 : 2 M Antenna Select  0 = Front, 1 = Rear
      //5 : UHF Antenna Select  0 = Front, 1 = Rear
      //6 : ? ?
      //7 : SPL On/Off  0 = Off, 1 = On
      *response = (globalSettings.splitOn ? 0xFF : 0x7F);
      break;
  }
}

//Maps some of the fixed memory layout of the FT817's EEPROM
void catReadEEPRom(uint8_t* cmd, uint8_t* response)
{
  const uint16_t read_address = cmd[P1] << 8 | cmd[P2];

  catGetEeprom(read_address,response);
  catGetEeprom(read_address+1,response+1);
}

void processCatCommand(uint8_t* cmd) {
  uint8_t response[FT817_MESSAGE_SIZE] = {0};
  uint8_t response_length = 0;

  switch(cmd[CMD]){
  case Ft817Command_e::SetFrequency:
  {
    uint32_t f = readFreq(cmd);
    setFrequency(f);
    updateDisplay();
    response[0] = 0x00;
    response_length = 1;
    break;
  }

  case Ft817Command_e::SplitOn:
    globalSettings.splitOn =  true;
    break;
  case Ft817Command_e::SplitOff:
    globalSettings.splitOn = false;
    break;

  case Ft817Command_e::ReadFreqAndMode:
    //First 4 bytes are the frequency
    writeFreq(GetActiveVfoFreq(),response);//bytes 0-3
    //Last byte is the mode
    if (VfoMode_e::VFO_MODE_USB == GetActiveVfoMode()){
      response[4] = OperatingMode_e::USB;
    }
    else{
      response[4] = OperatingMode_e::LSB;
    }
    Serial.write(response,5);
    break;

  case Ft817Command_e::OperatingMode:
    if(OperatingMode_e::LSB == cmd[P1] || OperatingMode_e::CWR == cmd[P1]){
      SetActiveVfoMode(VfoMode_e::VFO_MODE_LSB);
    }
    else{
      SetActiveVfoMode(VfoMode_e::VFO_MODE_USB);
    }
    response_length = 1;

    setFrequency(GetActiveVfoFreq());//Refresh frequency to get new mode to take effect
    updateDisplay();
    break;

  case Ft817Command_e::PttOn:
    if (!globalSettings.txActive) {
      globalSettings.txCatActive = true;
      startTx(globalSettings.tuningMode);
    }
    else {
      response[0] = 0xF0;
    }
    response_length = 1;
    updateDisplay();
    break;

  case Ft817Command_e::PttOff:
    if (globalSettings.txActive) {
      stopTx();
    }
    globalSettings.txCatActive = false;
    response_length = 1;
    updateDisplay();
    break;

  case Ft817Command_e::VfoToggle:
    if (Vfo_e::VFO_A == globalSettings.activeVfo){
      globalSettings.activeVfo = Vfo_e::VFO_B;
    }
    else{
      globalSettings.activeVfo = Vfo_e::VFO_A;
    }
    response_length = 1;
    updateDisplay();
    break;

  case Ft817Command_e::ReadEeprom:
    catReadEEPRom(cmd,response);
    response_length = 2;
    break;

  case Ft817Command_e::ReadRxStatus:
    //We don't have visibility into these values, so just hard code stuff
    ReadRxStatus_t reply_status;
    reply_status.Dummy = 0;
    reply_status.Smeter = 9;//S9
    reply_status.SquelchSuppressionActive = 0;
    reply_status.DiscriminatorCenteringOff = 1;
    reply_status.CodeUnmatched = 0;
    response[0] = *(uint8_t*)&reply_status;
    response_length = 1;
    break;

  case Ft817Command_e::ReadTxStatus:
  {
    //We don't have visibility into some of these values, so just hard code stuff
    ReadTxStatus_t reply_status;
    reply_status.Dummy = 0;
    reply_status.HighSwrDetected = 0;
    reply_status.PowerOutputMeter = 0xF;
    reply_status.PttOff = !globalSettings.txActive;
    reply_status.SplitOff = !globalSettings.splitOn;

    response[0] = *(uint8_t*)&reply_status;
    response_length = 1;
    break;
  }

  default:
    response_length = 1;
    break;
  }

  Serial.write(response, response_length);
}

void checkCAT(){
  static uint8_t rx_buffer[FT817_MESSAGE_SIZE];
  static uint8_t current_index = 0;
  static uint32_t timeout = 0;

  //Check Serial Port Buffer
  if (Serial.available() == 0) {      //Set Buffer Clear status
    if(timeout < millis()){
      current_index = 0;
      timeout = 0;
    }
    return;
  }
  else{
    if(0 == current_index){
      timeout = millis() + CAT_RECEIVE_TIMEOUT_MS;
    }
    rx_buffer[current_index] = Serial.read();
    ++current_index;
    if(current_index < FT817_MESSAGE_SIZE){
      return;
    }
  }

  processCatCommand(rx_buffer);
  current_index = 0;
  timeout = 0;
}


