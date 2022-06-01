# Firmware And Userland For HERMES Radio with GPS-corrected PLL

This repository contains the firmware and userland tools for the uBitx v6
based Rhizomatica's HF radio transceiver with GPS.

## Compile And Install

To compile the projects, run "make" for compiling the userland, and "make
firmware" for compiling the firmware. To install the firmware, run "make
ispload", and to install the tools, run "make install".

## Firmware Details

### Ubitx v6 connector pin assignments

*      Pin 1 (Violet), A7, REF MEASURE input
*      Pin 2 (Blue),   A6, FWD MEASURE input
*      Pin 3 (Green), +5v
*      Pin 4 (Yellow), GND
*      Pin 5 (Orange), A3, SYSTEM LED output
*      Pin 6 (Red),    A2, CONNECTED LED output
*      Pin 7 (Brown),  A1, ANT HIGH SWR RED LED output
*      Pin 8 (Black),  A0, ANT GOOD GREEN LED output

### Arduino pin assignments

D5 and D2 with changes for GPS calibration! LPF_A always off right now
(could use any other D pin to re-enable it), and CW_KEY also off (could also
use any other pin). TX_LPF_A and CW_KEY connections from Raduino to main
radio board should be:

*      D12, TX_LPF_A,      LPF_A (re-routed!)
*      D11, CW_KEY,          Pin goes high during CW keydown to transmit the carrier. (re-routed!)
*      D7, TX_RX,             Pin from the Nano to the radio to switch to TX (HIGH) and RX(LOW)
*      D6, CW_TONE,     Generates a square wave sidetone while sending the CW
*      D5, CAL_CLK,        CLK #0 connects here for calibration purposes (original TX_LPF_A disconnected - cut the trace in board)
*      D4, TX_LPF_B,      LPF_B
*      D3, TX_LPF_C,       LPF_C
*      D2, PPS_IN,              GPS 1PPS input  (original CW_KEY  disconnected - cut trace in board)

Please refer to this picture for all the mods needed for the HERMES
transceiver: https://github.com/DigitalHERMES/rhizo-transceiver/raw/main/ubitxv6_mods/hermes_raduino-mods.png
