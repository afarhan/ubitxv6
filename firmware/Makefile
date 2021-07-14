# use 'make' to build the source
# and 'make ispload' to upload to the arduino nano

# Set this to a recent Arduino release
ARDUINO_DIR = /usr/share/arduino/
#ARDUINO_DIR = /home/rafael2k/files/rhizomatica/hermes/arduino-1.8.13

BOARD_TAG = nano
BOARD_SUB = atmega328

ISP_PORT = /dev/ttyUSB0
ISP_PROG = arduino

AVRDUDE_ARD_BAUDRATE = 57600
AVRDUDE_OPTS = -q -V -D

# MONITOR_BAUDRATE=38400
# MONITOR_PORT=/dev/ttyUSB0

include /usr/share/arduino/Arduino.mk
