#!/bin/sh

echo ptt_on test
./ubitx_client -c ptt_on
./ubitx_client -c get_txrx_status

echo ptt_off test
./ubitx_client -c ptt_off
./ubitx_client -c get_txrx_status

echo frequency test
./ubitx_client -c set_frequency -a 6000000
./ubitx_client -c get_frequency
./ubitx_client -c set_frequency -a 7100000
./ubitx_client -c get_frequency

echo mode test
./ubitx_client -c set_mode -a LSB
./ubitx_client -c get_mode
./ubitx_client -c set_mode -a USB
./ubitx_client -c get_mode

echo get_protection_status
./ubitx_client -c get_protection_status

echo get_mastercal
./ubitx_client -c get_mastercal
# ./ubitx_client -c set_mastercal -a xxxxxx

echo get_bfo
./ubitx_client -c get_bfo
# ./ubitx_client -c set_bfo -a xxxxxx

echo get_fwd
./ubitx_client -c get_fwd

echo get_ref
./ubitx_client -c get_ref

echo get_ref
./ubitx_client -c get_ref

echo led tests
./ubitx_client -c set_led_status -a ON
./ubitx_client -c get_led_status
./ubitx_client -c set_led_status -a OFF
./ubitx_client -c get_led_status

echo bypass tests
./ubitx_client -c set_bypass_status -a ON
./ubitx_client -c get_bypass_status
./ubitx_client -c set_bypass_status -a OFF
./ubitx_client -c get_bypass_status

