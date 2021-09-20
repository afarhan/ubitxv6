# Firmware And Userland For HERMES Radio

This repository contains the firmware and userland tools for the uBitx v6
based Rhizomatica's HF radio transceiver.

## Organization

This repository is organized in directories, as follows:

* firmware: Contains the Arduino Nano radio firmware code.
* tools: Contains the userland daemon (ubitx_controller) and command line tool for radio control (ubitx_client).
* common: Contains shared code between firmware and userland.

The firmware for the older hardware revision using a discrete protection board logic is
present in:

* firmware-with_protection_board: Please read README inside directory.

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
*      Pin 6 (Red),    A2, BY-PASS CONTROL output
*      Pin 7 (Brown),  A1, ANT HIGH SWR RED LED output
*      Pin 8 (Black),  A0, ANT GOOD GREEN LED output

## Userland Details

## ubitx_client commands

Syntax:
    ubitx_client -c command [-a argument]

Examples:
    ubitx_client -c set_frequency -a 7100000
    ubitx_client -c get_frequency

Some commands need the argument parameters, while some don't. Following is a
list of all commands provided by the ubitx_client. The commands are followed
by the argument type and possible responses.

* ptt_on
  * No Argument
  * Resp: OK | NOK | SWR | ERROR

* ptt_off
  * No Argument
  * Resp: OK | NOK | SWR | ERROR

* get_frequency
  * No Argument
  * Resp: Frequency | ERROR

* set_frequency
  * Frequency
  * Resp: OK | ERROR

* get_mode
  * No Argument
  * Resp: USB | LSB | ERROR

* set_mode
  * LSB | USB
  * Resp: OK | ERROR

* get_txrx_status
  * No Argument
  * Resp: INTX | INRX | ERROR

* get_protection_status
  * No Argument
  * Resp: PROTECTION_ON | PROTECTION_OFF | ERROR

* get_mastercal
  * No Argument
  * Resp: Frequency | ERROR

* set_mastercal
  * Frequency
  * Resp: OK | ERROR

* get_bfo
  * No Argument
  * Resp: Frequency | ERROR

* set_bfo
  * Frequency
  * Resp: OK | ERROR

* get_fwd
  * No Argument
  * Resp: Power | ERROR

* get_ref
  * No Argument
  * Resp: Power | ERROR

* get_led_status
  * No Argument
  * Resp: LED_ON | LED_OFF | ERROR

* set_led_status
  * 0 | 1
  * Resp: OK | ERROR

* get_bypass_status
  * No Argument
  * Resp: BYPASS_ON | BYPASS_OFF | ERROR

* set_bypass_status
  * 0 | 1
  * Resp: OK | ERROR

* get_serial
  * No Argument
  * Resp: Serial Number | ERROR

* set_serial
  * Serial Number
  * Resp: OK | ERROR

* reset_protection
  * No Argument
  * Resp: OK | ERROR

* set_ref_threshold
  * Reflected Threshold Level For Protection Activation (0 - 1023)
  * Resp: OK | ERROR

* get_ref_threshold
  * No Argument
  * Resp: Reflected Threshold Level | ERROR

## Author

Rafael Diniz <rafael@riseup.net>

## License

GPLv3
