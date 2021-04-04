/* Rhizomatica Firmware for uBitx based Kurupira-1
 *
 * Copyright (C) 2021 Rhizomatica
 * Author: Rafael Diniz <rafael@riseup.net>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 */


#include <Arduino.h>
#include "ubitx.h"
#include "ubitx_cat.h"

/**
 * The CAT protocol is ad-hoc, make for Rhizomatica's Kurupira 1 radio,
 * which provide remote control to computers through the serial port.
 *
 */


static uint32_t rxBufferArriveTime = 0;
static uint8_t rxBufferCheckCount = 0;

static uint8_t cat[5];
static uint8_t insideCat = 0;

uint16_t skipTimeCount = 0;


void processCATCommand(byte* cmd)
{
    uint8_t response[5];

    switch(cmd[4]){
    case 0x07: // set mode
        if (cmd[0] == 0x00 || cmd[0] == 0x03)
            isUSB = 0;
        else
            isUSB = 1;

        saveVFOs();
        setFrequency(frequency);

        response[0] = ACK;
        Serial.write(response, 1);
        break;

  case 0x08: // PTT On
    if (!inTx) {
        response[0] = ACK;
        startTx();
    } else {
        response[0] = NACK;
    }
    Serial.write(response,1);
    break;

  case 0x88 : // PTT OFF
      if (inTx) {
          response[0] = ACK;
          stopTx();
      }
      else {
          response[0] = NACK;
      }
      Serial.write(response,1);
      break;

  case 0xf0: // GET SSB MODE
      // TODO
      break;

  case 0xf1: // GET TX/RX STATUS
      // TODO
      break;

  case 0xf2: // GET PROTECTION STATUS
      // TODO
      break;

  case 0xf3: // GET FWD
      // TODO
      break;

  case 0xf4: // GET REF
      // TODO
      break;

  case 0xf5: // SET LED STATUS
      // TODO
      break;

  case 0xf6: // GET LED STATUS
      // TODO
      break;


  case 0xf8: // GET BYPASS STATUS 
      response[0] = LONG_ACK;
      response[1] = by_pass;
      Serial.write(response,5);
      break;

  case 0xf9: // SET BYPASS STATUS
      by_pass = cmd[0];
      // setBypass()
      response[0] = ACK;
      Serial.write(response,1);
      // TODO
      break;

  case 0xfa: // SET FREQUENCY
      memcpy(&frequency, cmd, 4);
      setFrequency(frequency);
      saveVFOs();
      response[0] = ACK;
      Serial.write(response,1);
      break;

  case 0xfb: // GET FREQUENCY
      response[0] = LONG_ACK;
      memcpy(response+1, &frequency, 4);
      Serial.write(response,5);
      break;

  case 0xfc: // SET BFO
      memcpy(&usbCarrier, cmd, 4);
      setBFO(usbCarrier);
      response[0] = ACK;
      Serial.write(response,1);
      break;

  case 0xfd: // SET MASTER CAL
      memcpy(&calibration, cmd, 4);
      setMasterCal(calibration);
      response[0] = ACK;
      Serial.write(response,1);
      break;

  case 0xfe: // GET BFO
      response[0] = LONG_ACK;
      memcpy(response+1, &usbCarrier, 4);
      Serial.write(response,5);
      break;

  case 0xff: // GET MASTER CAL
      response[0] = LONG_ACK;
      memcpy(response+1, &calibration, 4);
      Serial.write(response,5);
      break;

  default:
      response[0] = NACK;
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


