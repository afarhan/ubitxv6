#include "cat.h"

#ifdef USE_CAT
#include <Arduino.h>

#include "display.h"
// #include "loop_master.h"
#include "radio.h"
#include "str_buffer.h"

const byte CAT_LOCK_ON   = 0x00;
const byte CAT_SET_FREQ  = 0x01;
const byte CAT_SPLIT_ON  = 0x02;
const byte CAT_FM_QUERY  = 0x03;
const byte CAT_RIT_ON    = 0x05;
const byte CAT_SET_MODE  = 0x07;
const byte CAT_PTT_ON    = 0x08;
const byte CAT_LOCK_OFF  = 0x80;
const byte CAT_TOGGLE_AB = 0x81;
const byte CAT_SPLIT_OFF = 0x82;
const byte CAT_RIT_OFF   = 0x85;
const byte CAT_PTT_OFF   = 0x88;
const byte CAT_RX_QUERY  = 0xe7;
const byte CAT_RIT_AMT   = 0xf5;
const byte CAT_TX_QUERY  = 0xf7;

// undocumented
const byte CAT_READ_EEPROM = 0xbb;
const byte CAT_READ_TX_METER = 0xbd;

byte cmd_buffer[5];
byte cmd_byte_ct = 0;

void CATcheck(void) {
  if (Serial.available()) {
    long frequency;
    byte temp1, temp2;
    int c = Serial.read();
    if (c > -1) {
      cmd_buffer[cmd_byte_ct++] = (byte) c;
    }
    if (cmd_byte_ct == 5) {
      cmd_byte_ct = 0;
      #ifdef USE_CAT_ACTIVITY
      disp.cat_activity(); 
      #endif
      byte cmd = cmd_buffer[4];
      switch (cmd) {
        #ifdef USE_LOCK
          case CAT_LOCK_ON:
          case CAT_LOCK_OFF: 
            radio_obj.toggle_lock(cmd == CAT_LOCK_ON);
            break;
        #endif

        case CAT_PTT_ON: 
        case CAT_PTT_OFF:
          radio_obj.set_ptt(cmd == CAT_PTT_ON);
          Serial.write(0x00);
          break;

        case CAT_SET_FREQ:
          frequency = 0;
          for (byte i=0; i<4; i++) {
            frequency += ((uint8_t) cmd_buffer[i] >> 4);
            frequency *= 10;
            frequency += ((uint8_t) cmd_buffer[i] & 0x0f); 
            frequency *= 10;
          }
          radio_obj.inc_step(true);     // reset freq step
          if (radio_obj.set_frequency(frequency)) {
            Serial.write(0x00);
          }
          break;

        case CAT_SET_MODE:
          temp1 = cmd_buffer[0];
          if (temp1 < 4) {
            radio_obj.toggle_cw((temp1 & 0b10));
            radio_obj.toggle_usb((temp1 & 0b01));
            Serial.write(0x00);
          }
          break;

        case CAT_RIT_ON:
        case CAT_RIT_OFF:
          break;

        case CAT_RIT_AMT:
          break;

        case CAT_TOGGLE_AB: 
          radio_obj.toggle_active_vfo();
          Serial.write(0x00);
          break;

        case CAT_SPLIT_ON: 
          radio_obj.toggle_split(true);
          Serial.write(0x00);
          break;

        case CAT_SPLIT_OFF: 
          radio_obj.toggle_split(false);
          Serial.write(0x00);        
          break;

        case CAT_RX_QUERY:
          Serial.write(0x00);
          break;

        case CAT_TX_QUERY:
          Serial.write(
            (radio_obj.split ? 0 : _BV(5))
            | (radio_obj.in_tx ? 0 : _BV(7))
          );
          break;

        case CAT_FM_QUERY:
          frequency = (radio_obj.vfo_data[radio_obj.using_vfo_b].frequency) / 10;
          str_buffer1[4] = radio_obj.vfo_data[radio_obj.using_vfo_b].usb
            ? (radio_obj.vfo_data[radio_obj.using_vfo_b].cw ? 0x03 : 0x01)
            : (radio_obj.vfo_data[radio_obj.using_vfo_b].cw ? 0x02 : 0x00);
          for (int8_t i=3; i>=0; i--) {
            temp1 = frequency  % 10;
            frequency /= 10;
            temp2 = frequency % 10;
            frequency /= 10;
            str_buffer1[i] = (temp2 << 4) | temp1; 
          }
          Serial.write(str_buffer1, 5);
          break;

        case CAT_READ_EEPROM:
          switch (str_buffer1[1]) {
            case 0x55:  // VFO A or B
              Serial.write(radio_obj.using_vfo_b ? 0x81 : 0x80);
              Serial.write(0x00);
              break;
            case 0x64:  // baud rate
              Serial.write(0x80);
              Serial.write(0x00);
              break;
            case 0x78:
              Serial.write(radio_obj.vfo_data[radio_obj.using_vfo_b].usb ? 0x20 : 0);
              Serial.write(0x00);
              break;
            case 0x7a:  // split
              Serial.write(radio_obj.split ? 0x80 : 0x00);
              Serial.write(0x00);
            default:
              Serial.write(0x00);
              Serial.write(0x00);
              break;
          }
          break;

        case CAT_READ_TX_METER:
          Serial.write(0x00);
          Serial.write(0x00);
          break;
      }
    }
  }
}
#endif