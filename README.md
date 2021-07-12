# uBitx v6 Arduino firmware for HERMES project

## Ubitx pin assignments

 *      Pin 1 (Violet), A7, REF MEASURE input
 *      Pin 2 (Blue),   A6, FWD MEASURE input
 *      Pin 3 (Green), +5v
 *      Pin 4 (Yellow), GND
 *      Pin 5 (Orange), A3, LED CONTROL output
 *      Pin 6 (Red),    A2, BY-PASS CONTROL output
 *      Pin 7 (Brown),  A1, SWR PROTECTION STATUS input
 *      Pin 8 (Black),  A0, SWR PROTECTION RESET output

## Ubitx commands

Following is a list of all commands provided by the ubitx_client. The commands are followed by the 
argument type and possible responses.

* ptt_on
  * No Argument
  * Resp: OK | NOK | SWR | ERROR

 * ptt_off No argument
  * No Argument
  * Resp: OK | NOK | ERROR

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

* get_ref

* get_led_status

* set_led_status

* get_bypass_status

* set_bypass_status

* get_serial

* set_serial

*reset_protection


## Author

Rafael Diniz <rafael@riseup.net>

## License

GPLv3
