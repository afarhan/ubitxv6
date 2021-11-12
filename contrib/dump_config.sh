#!/bin/sh
echo -n "freq "
ubitx_client -c get_frequency
echo -n "mode "
ubitx_client -c get_mode
echo -n "bfo "
ubitx_client -c get_bfo
echo -n "mastercal "
ubitx_client -c get_mastercal
echo -n "txrx "
ubitx_client -c get_txrx_status
echo -n "protection "
ubitx_client -c get_protection_status
echo -n "swr_threshold "
ubitx_client -c get_ref_threshold
echo -n "forward "
ubitx_client -c get_fwd
echo -n "reflected "
ubitx_client -c get_ref
echo -n "sys_led "
ubitx_client -c get_led_status
echo -n "connected_led "
ubitx_client -c get_bypass_status
echo -n "serial "
ubitx_client -c get_serial
