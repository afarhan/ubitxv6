# ubitxv6 firmware for HERMES project
Based on uBitx v6.3.1 Arduino code by Ashhar Farhan

- This uses pins A6 and A7 for forward and reflected references measures, shown in the screen instead of CW settings (to show click the new SWR button)
- Band filter settings are adjusted so that the transceiver works between 5 and 7 MHz
- Front-panel PTT is disabled by default (re-enable througn the new PTT button), as the plan is to use just CAT over serial
- CW Keyer is disabled, as we use A6 for SWR measurements

This is released under GPL v3 license.
