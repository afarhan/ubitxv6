#pragma once

// use the defines below to enable or disable features
// ---------------------------------------------------
#define USE_CAT             // CAT, subset of Yaesu FT817ND - works with WSJT-X and FLDigi hamlib
// #define USE_HAM_BAND     // band selection screen (ham bands)

// MEE add-ons
// #define USE_BC_BAND      // band selection screen (broadcast bands)
#define USE_MEMORY          // M1 .. M5 buttons
// #define USE_RF_SHIFT     // RF shift - provides an adjustable RF shift
#define USE_RF_SHIFT_STEP   // RF shift - provides stepped settings of 0, -10dB, -20dB, -30dB
#define USE_ATTN         // RF Attenuator - based on reducing drive level of 1st IF
#define USE_LOCK            // frequency lock
#define USE_IF_SHIFT        // IF shift - tone control
#define USE_SPOT            // CW spotting tone
#define USE_TUNE            // Tune output
#define USE_PDL_POL         // paddle polarity
#define USE_TX_DIS          // Tx disable button
#define USE_HB_DISP         // Flashing HB indicator
// #define USE_CAT_ACTIVITY // Flashing CAT activity indicator
// #define USE_MAN_SAVE     // manually save VFOs
#define USE_AUTO_SAVE       // Periodic auto-save VFOs 
#define USE_WINKEY          // Winkeyer (CW - tip, PTT - ring) option on paddle jack
// #define USE_RX_ONLY         // disable all TX functions
