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

static unsigned long rxBufferArriveTime = 0;
static uint8_t rxBufferCheckCount = 0;

static uint8_t cat[5];//Data is ordered parameters 1-4, then command code last
enum CatDataIndex_e : uint8_t {
  P1 = 0,
  P2 = 1,
  P3 = 2,
  P4 = 3,
  CMD = 4
};

static uint8_t insideCat = 0; 

//for broken protocol
static const uint16_t CAT_RECEIVE_TIMEOUT = 500;

static const uint8_t CAT_MODE_LSB           = 0x00;
static const uint8_t CAT_MODE_USB           = 0x01;
static const uint8_t CAT_MODE_CW            = 0x02;
static const uint8_t CAT_MODE_CWR           = 0x03;
static const uint8_t CAT_MODE_AM            = 0x04;
static const uint8_t CAT_MODE_FM            = 0x08;
static const uint8_t CAT_MODE_DIG           = 0x0A;
static const uint8_t CAT_MODE_PKT           = 0x0C;
static const uint8_t CAT_MODE_FMN           = 0x88;

static const uint8_t ACK = 0;

unsigned int skipTimeCount = 0;

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
  cmd[3] = setLowNibble(cmd[3],digits[1]);
  cmd[3] = setHighNibble(cmd[3],digits[2]);
  cmd[2] = setLowNibble(cmd[2],digits[3]);
  cmd[2] = setHighNibble(cmd[2],digits[4]);
  cmd[1] = setLowNibble(cmd[1],digits[5]);
  cmd[1] = setHighNibble(cmd[1],digits[6]);
  cmd[0] = setLowNibble(cmd[0],digits[7]);
  cmd[0] = setHighNibble(cmd[0],digits[8]);  
}

// This function takes a frquency that is encoded using 4 uint8_ts of BCD
// representation and turns it into an long measured in Hz.
//
// [12][34][56][78] = 123.45678? Mhz
//
unsigned long readFreq(uint8_t* cmd) {
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

//void ReadEEPRom_FT817(uint8_t fromType)
void catReadEEPRom(void)
{
  //for remove warnings
  uint8_t temp0 = cat[P1];
  uint8_t temp1 = cat[P2];

  cat[P1] = 0;
  cat[P2] = 0;
  //for remove warnings[1] = 0;
  
  switch (temp1)
  {
    case 0x45 : //
      if (temp0 == 0x03)
      {
        cat[P1] = 0x00;
        cat[P2] = 0xD0;
      }
      break;
    case 0x47 : //
      if (temp0 == 0x03)
      {
        cat[P1] = 0xDC;
        cat[P2] = 0xE0;
      }
      break;
    case 0x55 :
      //0 : VFO A/B  0 = VFO-A, 1 = VFO-B
      //1 : MTQMB Select  0 = (Not MTQMB), 1 = MTQMB ("Memory Tune Quick Memory Bank")
      //2 : QMB Select  0 = (Not QMB), 1 = QMB  ("Quick Memory Bank")
      //3 :
      //4 : Home Select  0 = (Not HOME), 1 = HOME memory
      //5 : Memory/MTUNE select  0 = Memory, 1 = MTUNE
      //6 :
      //7 : MEM/VFO Select  0 = Memory, 1 = VFO (A or B - see bit 0)
      cat[P1] = 0x80 + ((VFO_B == globalSettings.activeVfo) ? 1 : 0);
      cat[P2] = 0x00;
      break;
    case 0x57 : //
      //0 : 1-0  AGC Mode  00 = Auto, 01 = Fast, 10 = Slow, 11 = Off
      //2  DSP On/Off  0 = Off, 1 = On  (Display format)
      //4  PBT On/Off  0 = Off, 1 = On  (Passband Tuning)
      //5  NB On/Off 0 = Off, 1 = On  (Noise Blanker)
      //6  Lock On/Off 0 = Off, 1 = On  (Dial Lock)
      //7  FST (Fast Tuning) On/Off  0 = Off, 1 = On  (Fast tuning)

      cat[P1] = 0xC0;
      cat[P2] = 0x40;
      break;
    case 0x59 : //  band select VFO A Band Select  0000 = 160 M, 0001 = 75 M, 0010 = 40 M, 0011 = 30 M, 0100 = 20 M, 0101 = 17 M, 0110 = 15 M, 0111 = 12 M, 1000 = 10 M, 1001 = 6 M, 1010 = FM BCB, 1011 = Air, 1100 = 2 M, 1101 = UHF, 1110 = (Phantom)
      //http://www.ka7oei.com/ft817_memmap.html
      //CAT_BUFF[0] = 0xC2;
      //CAT_BUFF[1] = 0x82;
      break;
    case 0x5C : //Beep Volume (0-100) (#13)
      cat[P1] = 0xB2;
      cat[P2] = 0x42;
      break;
    case 0x5E :
      //3-0 : CW Pitch (300-1000 Hz) (#20)  From 0 to E (HEX) with 0 = 300 Hz and each step representing 50 Hz
      //5-4 :  Lock Mode (#32) 00 = Dial, 01 = Freq, 10 = Panel
      //7-6 :  Op Filter (#38) 00 = Off, 01 = SSB, 10 = CW
      //CAT_BUFF[0] = 0x08;
      cat[P1] = (globalSettings.cwSideToneFreq - 300)/50;
      cat[P2] = 0x25;
      break;
    case 0x61 : //globalSettings.cwSideToneFreq (Volume) (#44)
      cat[P1] = globalSettings.cwSideToneFreq % 50;
      cat[P2] = 0x08;
      break;
    case  0x5F : //
      //4-0  CW Weight (1.:2.5-1:4.5) (#22)  From 0 to 14 (HEX) with 0 = 1:2.5, incrementing in 0.1 weight steps
      //5  420 ARS (#2)  0 = Off, 1 = On
      //6  144 ARS (#1)  0 = Off, 1 = On
      //7  Sql/RF-G (#45)  0 = Off, 1 = On
      cat[P1] = 0x32;
      cat[P2] = 0x08;
      break;
    case 0x60 : //CW Delay (10-2500 ms) (#17)  From 1 to 250 (decimal) with each step representing 10 ms
      cat[P1] = globalSettings.cwActiveTimeoutMs / 10;
      cat[P2] = 0x32;
      break;
    case 0x62 : //
      //5-0  CW Speed (4-60 WPM) (#21) From 0 to 38 (HEX) with 0 = 4 WPM and 38 = 60 WPM (1 WPM steps)
      //7-6  Batt-Chg (6/8/10 Hours (#11)  00 = 6 Hours, 01 = 8 Hours, 10 = 10 Hours
      //CAT_BUFF[0] = 0x08;
      cat[P1] = 1200 / globalSettings.cwDitDurationMs - 4;
      cat[P2] = 0xB2;
      break;
    case 0x63 : //
      //6-0  VOX Gain (#51)  Contains 1-100 (decimal) as displayed
      //7  Disable AM/FM Dial (#4) 0 = Enable, 1 = Disable
      cat[P1] = 0xB2;
      cat[P2] = 0xA5;
      break;
    case 0x64 : //
      break;
    case 0x67 : //6-0  SSB Mic (#46) Contains 0-100 (decimal) as displayed
      cat[P1] = 0xB2;
      cat[P2] = 0xB2;
      break;
    case 0x69 : //FM Mic (#29)  Contains 0-100 (decimal) as displayed
    case 0x78 :
      if (VfoMode_e::VFO_MODE_USB == GetActiveVfoMode())
        cat[P1] = CAT_MODE_USB << 5;
      else
        cat[P1] = CAT_MODE_LSB << 5;
      break;
    case  0x79 : //
      //1-0  TX Power (All bands)  00 = High, 01 = L3, 10 = L2, 11 = L1
      //3  PRI On/Off  0 = Off, 1 = On
      //DW On/Off  0 = Off, 1 = On
      //SCN (Scan) Mode  00 = No scan, 10 = Scan up, 11 = Scan down
      //ART On/Off  0 = Off, 1 = On
      cat[P1] = 0x00;
      cat[P2] = 0x00;
      break;
    case 0x7A : //SPLIT
      //7A  0 HF Antenna Select 0 = Front, 1 = Rear
      //7A  1 6 M Antenna Select  0 = Front, 1 = Rear
      //7A  2 FM BCB Antenna Select 0 = Front, 1 = Rear
      //7A  3 Air Antenna Select  0 = Front, 1 = Rear
      //7A  4 2 M Antenna Select  0 = Front, 1 = Rear
      //7A  5 UHF Antenna Select  0 = Front, 1 = Rear
      //7A  6 ? ?
      //7A  7 SPL On/Off  0 = Off, 1 = On

      cat[P1] = (globalSettings.splitOn ? 0xFF : 0x7F);
      break;
    case 0xB3 : //
      cat[P1] = 0x00;
      cat[P2] = 0x4D;
      break;

  }

  // sent the data
  Serial.write(cat, 2);
}

void processCATCommand2(uint8_t* cmd) {
  uint8_t response[5];
  unsigned long f;
  
  switch(cmd[4]){
/*  case 0x00:
    response[0]=0;
    Serial.write(response, 1);
    break;
*/    
  case 0x01:
    //set frequency
    f = readFreq(cmd);
    setFrequency(f);   
    updateDisplay();
    response[0]=0;
    Serial.write(response, 1);
    //sprintf(b, "set:%ld", f); 
    //printLine2(b);
    break;

  case 0x02:
    //split on 
    globalSettings.splitOn =  1;
    break;
  case 0x82:
    //split off
    globalSettings.splitOn = 0;
    break;
    
  case 0x03:
    writeFreq(GetActiveVfoFreq(),response); // Put the frequency into the buffer
    if (VfoMode_e::VFO_MODE_USB == GetActiveVfoMode())
      response[4] = 0x01; //USB
    else
      response[4] = 0x00; //LSB
    Serial.write(response,5);
    //printLine2("cat:getfreq");
    break;
    
  case 0x07: // set mode
    if (cmd[0] == 0x00 || cmd[0] == 0x03)
      SetActiveVfoMode(VfoMode_e::VFO_MODE_LSB);
    else
      SetActiveVfoMode(VfoMode_e::VFO_MODE_USB);
    response[0] = 0x00;
    Serial.write(response, 1);
    setFrequency(GetActiveVfoFreq());
      //printLine2("cat: mode changed");
    //updateDisplay();
    break;   
 
  case 0x08: // PTT On
    if (!globalSettings.txActive) {
      response[0] = 0;
      globalSettings.txCatActive = true;
      startTx(TuningMode_e::TUNE_SSB);
      updateDisplay();
    } else {
      response[0] = 0xf0;
    } 
    Serial.write(response,1);
    updateDisplay();
    break;

  case 0x88 : //PTT OFF
    if (globalSettings.txActive) {
      stopTx();
      globalSettings.txCatActive = false;
    }
    response[0] = 0;
    Serial.write(response,1);
    updateDisplay();
    break;

  case 0x81:
    //toggle the VFOs
    response[0] = 0;
    if (Vfo_e::VFO_A == globalSettings.activeVfo){
      globalSettings.activeVfo = Vfo_e::VFO_B;
    }
    else{
      globalSettings.activeVfo = Vfo_e::VFO_A;
    }
    Serial.write(response,1);
    updateDisplay();
    break;

 case 0xBB:  //Read FT-817 EEPROM Data  (for comfirtable)
    catReadEEPRom();
    break;

  case 0xe7 : 
    // get receiver status, we have hardcoded this as
    //as we dont' support ctcss, etc.
    response[0] = 0x09;
    Serial.write(response,1);
    break;
       
  case 0xf7:
    {
      boolean isHighSWR = false;
      boolean issplitOn = false;
  
      /*
        Inverted -> *ptt = ((p->tx_status & 0x80) == 0); <-- souce code in ft817.c (hamlib)
      */
      response[0] = ((globalSettings.txActive ? 0 : 1) << 7) +
        ((isHighSWR ? 1 : 0) << 6) +  //hi swr off / on
        ((issplitOn ? 1 : 0) << 5) + //Split on / off
        (0 << 4) +  //dummy data
        0x08;  //P0 meter data

      Serial.write(response, 1);
    }
    break;
    
  default:
    //somehow, get this to print the four uint8_ts
    ultoa(*((unsigned long *)cmd), c, 16);
    response[0] = 0x00;
    Serial.write(response[0]);
  }

  insideCat = false;
}

int catCount = 0;
void checkCAT(){
  uint8_t i;

  //Check Serial Port Buffer
  if (Serial.available() == 0) {      //Set Buffer Clear status
    rxBufferCheckCount = 0;
    return;
  }
  else if (Serial.available() < 5) {                         //First Arrived
    if (rxBufferCheckCount == 0){
      rxBufferCheckCount = Serial.available();
      rxBufferArriveTime = millis() + CAT_RECEIVE_TIMEOUT;  //Set time for timeout
    }
    else if (rxBufferArriveTime < millis()){                //Clear Buffer
      for (i = 0; i < Serial.available(); i++)
        rxBufferCheckCount = Serial.read();
      rxBufferCheckCount = 0;
    }
    else if (rxBufferCheckCount < Serial.available()){      // Increase buffer count, slow arrive
      rxBufferCheckCount = Serial.available();
      rxBufferArriveTime = millis() + CAT_RECEIVE_TIMEOUT;  //Set time for timeout
    }
    return;
  }

    
  //Arived CAT DATA
  for (i = 0; i < 5; i++)
    cat[i] = Serial.read();


  //this code is not re-entrant.
  if (insideCat == 1)
    return;
  insideCat = 1;

/**
 *  This routine is enabled to debug the cat protocol
**/
  catCount++;

/*
  if (cat[CMD] != 0xf7 && cat[CMD] != 0xbb && cat[CMD] != 0x03){
    sprintf(b, "%d %02x %02x%02x%02x%02x", catCount, cat[CMD],cat[P1], cat[P2], cat[P3], cat[P4]);  
    printLine2(b);  
  }
*/  

/*
  if (!doingCAT){
    doingCAT = 1;
    displayText("CAT on", 100,120,100,40, ILI9341_ORANGE, ILI9341_BLACK, ILI9341_WHITE);
  }
*/
  processCATCommand2(cat);
  insideCat = 0;
}


