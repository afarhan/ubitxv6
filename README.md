# ubitxv6 firmware for HERMES project
Based on Ashhar Farhan Arduino code, forked by Reed N.

- This uses pins A6 and A7 for forward and reflected references measures, shown in the screen instead of CW settings (to show click the new SWR button)
- Band filter settings are adjusted so that the transceiver works between 5 and 7 MHz
- Front-panel PTT is disabled by default (re-enable througn the new PTT button), as the plan is to use just CAT over serial
- CW Keyer is disabled, as we use A6 for SWR measurements

# License

The majority of this code is released under GPL v3 license, per Ashhar's original code.

The files in the PDQ_MinLib subdirectory were copied from https://github.com/XarkLabs/PDQ_GFX_Libs, and have Adafruit's BSD License.

The files in the toneAC2 were copied from https://bitbucket.org/teckel12/arduino-toneac2/wiki/Home, and are GPL v3 licensed.
