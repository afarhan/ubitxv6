#include <Arduino.h>
#include "ubitx.h"

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
static byte rxBufferCheckCount = 0;
#define CAT_RECEIVE_TIMEOUT 500
static byte cat[5];
static byte insideCat = 0;

//for broken protocol
#define CAT_RECEIVE_TIMEOUT 500

#define CAT_MODE_LSB            0x00
#define CAT_MODE_USB            0x01
#define CAT_MODE_CW             0x02
#define CAT_MODE_CWR            0x03
#define CAT_MODE_AM             0x04
#define CAT_MODE_FM             0x08
#define CAT_MODE_DIG            0x0A
#define CAT_MODE_PKT            0x0C
#define CAT_MODE_FMN            0x88

#define ACK 0

unsigned int skipTimeCount = 0;

byte setHighNibble(byte b,byte v) {
  // Clear the high nibble
  b &= 0x0f;
  // Set the high nibble
  return b | ((v & 0x0f) << 4);
}

byte setLowNibble(byte b,byte v) {
  // Clear the low nibble
  b &= 0xf0;
  // Set the low nibble
  return b | (v & 0x0f);
}

byte getHighNibble(byte b) {
  return (b >> 4) & 0x0f;
}

byte getLowNibble(byte b) {
  return b & 0x0f;
}

// Takes a number and produces the requested number of decimal digits, staring
// from the least significant digit.  
//
void getDecimalDigits(unsigned long number,byte* result,int digits) {
  for (int i = 0; i < digits; i++) {
    // "Mask off" (in a decimal sense) the LSD and return it
    result[i] = number % 10;
    // "Shift right" (in a decimal sense)
    number /= 10;
  }
}

// Takes a frequency and writes it into the CAT command buffer in BCD form.
//
void writeFreq(unsigned long freq, byte* cmd) {
  // Convert the frequency to a set of decimal digits. We are taking 9 digits
  // so that we can get up to 999 MHz. But the protocol doesn't care about the
  // LSD (1's place), so we ignore that digit.
  byte digits[9];
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

// This function takes a frquency that is encoded using 4 bytes of BCD
// representation and turns it into an long measured in Hz.
//
// [12][34][56][78] = 123.45678? Mhz
//
unsigned long readFreq(byte* cmd) {
    // Pull off each of the digits
    byte d7 = getHighNibble(cmd[0]);
    byte d6 = getLowNibble(cmd[0]);
    byte d5 = getHighNibble(cmd[1]);
    byte d4 = getLowNibble(cmd[1]); 
    byte d3 = getHighNibble(cmd[2]);
    byte d2 = getLowNibble(cmd[2]); 
    byte d1 = getHighNibble(cmd[3]);
    byte d0 = getLowNibble(cmd[3]); 
    return
        (unsigned long)d7 * 100000000L +
        (unsigned long)d6 * 10000000L +
        (unsigned long)d5 * 1000000L + 
        (unsigned long)d4 * 100000L + 
        (unsigned long)d3 * 10000L + 
        (unsigned long)d2 * 1000L + 
        (unsigned long)d1 * 100L + 
        (unsigned long)d0 * 10L; 
}

//void ReadEEPRom_FT817(byte fromType)
void catReadEEPRom(void)
{
    
    // sent the data
    Serial.write(cat, 2);
}

void processCATCommand(byte* cmd) {
  byte response[5];
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
      saveVFOs();
      response[0]=0;
      Serial.write(response, 1);
      break;

  case 0x03: // read freq
      writeFreq(frequency,response); // Put the frequency into the buffer
      if (isUSB)
          response[4] = 0x01; //USB
      else
          response[4] = 0x00; //LSB
      Serial.write(response,5);
      break;

  case 0x07: // set mode
    if (cmd[0] == 0x00 || cmd[0] == 0x03)
        isUSB = 0;
    else
        isUSB = 1;

    response[0] = 0x00;
    Serial.write(response, 1);
    setFrequency(frequency);

    break;

  case 0x08: // PTT On
    if (!inTx) {
        response[0] = 0;
        startTx();
    } else {
        response[0] = 0xf0;
    }
    Serial.write(response,1);
    break;

  case 0x88 : //PTT OFF
    if (inTx) {
        response[0] = 0;
        stopTx();
    }
    response[0] = 0;
    Serial.write(response,1);
    break;

 case 0xBB:  //Read FT-817 EEPROM Data  (for comfirtable)
    catReadEEPRom();
    break;

  case 0xE7:
    // get receiver status, we have hardcoded this as
    //as we dont' support ctcss, etc.
    response[0] = 0x09;
    Serial.write(response,1);
    break;

  case 0xf7:
    {
      /*
        Inverted -> *ptt = ((p->tx_status & 0x80) == 0); <-- souce code in ft817.c (hamlib)
      */
      response[0] = ((inTx ? 0 : 1) << 7) +
        ((isHighSWR ? 1 : 0) << 6) +  //hi swr off / on
//        ((isSplitOn ? 1 : 0) << 5) + //Split on / off
        (0 << 4) +  //dummy data
        0x08;  //P0 meter data

      Serial.write(response, 1);
    }
    break;
  default:
      response[0] = 0xf0;
      Serial.write(response, 1);
  }

}

int catCount = 0;
void checkCAT(){
  byte i;

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

  // So here we lose data??? not ok....
  //this code is not re-entrant.
  // if (insideCat == 1)
  //    return;
  insideCat = 1;

  catCount++;

  processCATCommand(cat);
  insideCat = 0;
}


