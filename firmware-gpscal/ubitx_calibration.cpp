/*
    uBitx v6 HERMES firmware
    Copyright (C) 2022 Rhizomatica <rafael@rhizomatica.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    ubitx_calibration.cpp

*/

#include "ubitx_calibration.h"

bool GpsOneSecTick = false;
uint16_t tcount = 2;
uint32_t mult=0;

uint32_t XtalFreq=0;

void PPSinterrupt()
{
    GpsOneSecTick = true;                        // New second by GPS.

    tcount++;
    if (tcount == 4)                               // Start counting the xxx MHz signal from Si5351A CLK0
    {
        TCCR1B = 7;                                  //Clock on rising edge of pin 5
    }
    else if (tcount == 44)                         //The 40 second gate time elapsed - stop counting
    {
        TCCR1B = 0;                                  //Turn off counter
        XtalFreq = mult * 0x10000 + TCNT1;          //Calculate correction factor, eg. 5Mhz: multi * 65536 + ... =(deve ser)4= 5 MHz * 40 (s)
        TCNT1 = 0;                                   //Reset count to zero
        mult = 0;
        tcount = 0;                                  //Reset the seconds counter
    }
}

// Timer 1 overflow intrrupt vector.
ISR(TIMER1_OVF_vect)
{
    mult++;                                          //Increment multiplier
    TIFR1 = (1<<TOV1);                               //Clear overlow flag 
}
