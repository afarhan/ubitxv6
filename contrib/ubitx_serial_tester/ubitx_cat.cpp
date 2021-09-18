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
#include "ubitx_cat.h"

/**
 * The CAT protocol is ad-hoc, make for Rhizomatica's Kurupira 1 radio,
 * which provide remote control to computers through the serial port.
 *
 */


static uint8_t cat;
static uint8_t insideCat = 0;

int catCount = 0;

void checkCAT(){

  //Check Serial Port Buffer
  if (Serial.available() == 0) {      //Set Buffer Clear status
    return;
  }

  cat = Serial.read();

  Serial.write(&cat,1);

  // So here we lose data??? not ok....
  //this code is not re-entrant.
  // if (insideCat == 1)
  //    return;
  insideCat = 1;

  catCount++;
  insideCat = 0;
}
