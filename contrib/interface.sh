#!/bin/bash
CMD="/usr/bin/ubitx_client "
CMD_SF="$CMD -c set_frequency -a "
CMD_GF="$CMD -c get_frequency"
CMD_GM="$CMD -c get_mode"
CMD_SM="$CMD -c set_mode -a "
CMD_GBFO="$CMD -c get_bfo"
CMD_SBFO="$CMD -c set_bfo -a "
CMD_GMASTERCAL="$CMD -c get_mastercal"
CMD_SMASTERCAL="$CMD -c set_mastercal -a"

FREQ=$($CMD_GF)
echo "Frequency: ${FREQ} Hz"

MODE=$($CMD_GM)
echo "MODE: ${MODE}"

BFO=$($CMD_GBFO)
echo "BFO: ${BFO} Hz"

MASTERCAL=$($CMD_GMASTERCAL)
echo "MasterCAL: ${MASTERCAL}"

escape_char=$(printf "\u1b")

echo "tips:"
echo "frequency:"
echo "+ + + + + +"
echo "a s d f h j "
echo "z x c v b n "
echo "- - - - - -"
echo "u - USB / l LSB"
echo "i - Insert Freq"

echo "+ -  : BFO"
echo "UP DOWN : MASTERCAL"

while true; do
read -rsn1 INPUT

if [[ $INPUT == $escape_char ]]; then
  read -rsn2 INPUT # read 2 more chars
fi

case $INPUT in

  a)
    echo -n "freq +1MHZ:    "
    FREQ=$(expr $FREQ + 1000000)
    echo -n "freq" $FREQ "Hz"
    $CMD_SF $FREQ
    ;;
  z)
    echo -n "freq -1MHZ:    "
    FREQ=$(expr $FREQ - 1000000)
    echo -n "freq" $FREQ "Hz"
    $CMD_SF $FREQ
    ;;

  s)
    echo -n "freq +100 KHz: "
    FREQ=$(expr $FREQ + 100000)
    echo -n "freq" $FREQ  "Hz"
    $CMD_SF $FREQ
    ;;

  x)
    echo -n "freq -100 KHz: "
    FREQ=$(expr $FREQ - 100000)
    echo -n "freq" $FREQ  " Hz"
    $CMD_SF $FREQ
    ;;
  d)
    echo -n "freq +10 KHz:  "
    FREQ=$(expr $FREQ + 10000)
    echo -n "freq" $FREQ  "Hz"
    $CMD_SF $FREQ
    ;;

  c)
    echo -n "freq -10 KHz:  "
    FREQ=$(expr $FREQ - 10000)
    echo -n "freq" $FREQ  "Hz"
    $CMD_SF $FREQ
    ;;
  f)
    echo -n "freq +1 KHz:   "
    FREQ=$(expr $FREQ + 1000)
    echo -n "freq" $FREQ  "Hz"
    $CMD_SF $FREQ
    ;;

  v)
    echo -n "freq -1 KHz:   "
    FREQ=$(expr $FREQ - 1000)
    echo -n "freq" $FREQ  "Hz"
    $CMD_SF $FREQ
    ;;

  g)
    echo -n "freq +100 Hz:  "
    FREQ=$(expr $FREQ + 100)
    echo -n "freq" $FREQ  "Hz"
    $CMD_SF $FREQ
    ;;

  b)
    echo -n "freq -100 Hz:  "
    FREQ=$(expr $FREQ - 100)
    echo -n "freq" $FREQ  "Hz"
    $CMD_SF $FREQ
    ;;

  h)
    echo -n "freq +10 Hz:   "
    FREQ=$(expr $FREQ + 10)
    echo -n "freq" $FREQ  "Hz"
    $CMD_SF $FREQ
    ;;

  n)
    echo -n "freq -10 HZ:   "
    FREQ=$(expr $FREQ - 10)
    echo -n "freq" $FREQ  "Hz"
    $CMD_SF $FREQ
    ;;

  j)
    echo -n "freq +1 Hz:    "
    FREQ=$(expr $FREQ + 1)
    echo -n "freq" $FREQ  "Hz"
    $CMD_SF $FREQ
    ;;

  m)
    echo -n "freq -1 Hz:    "
    FREQ=$(expr $FREQ - 1)
    echo -n "freq" $FREQ  "Hz"
    $CMD_SF $FREQ
    ;;

  u)
    echo  -n "set mode USB "
    $CMD_SM "USB"
    ;;
  l)
    echo  -n "set mode LSB "
    $CMD_SM "LSB"
    ;;


  1)
    echo  -n "bookmark 1: 10 meters 1 MHz = 1.000.000 Hz "
    FREQ=1000000
    echo -n "freq" $FREQ " "
    $CMD_SF $FREQ
    ;;
  2)
    echo  -n "bookmark 2: 20 meters 3 MHz = 3.000.000 Hz "
    FREQ=3000000
    echo -n "freq" $FREQ " "
    $CMD_SF $FREQ 
    ;;
  3)
    echo  -n "bookmark 3: 40 meters 6.98 MHz = 6.980.000 Hz "
    FREQ=3000000
    echo -n "freq" $FREQ " "
    $CMD_SF $FREQ 
    ;;
  4)
    echo  -n "bookmark 4: 40 meters 7 MHz = 7.000.000 Hz "
    FREQ=7000000
    echo -n "freq" $FREQ " "
    $CMD_SF $FREQ 
    ;;
  8)
    echo  -n "bookmark 8: 80 meters 14 MHz = 14.000.000 Hz "
    FREQ=7000000
    echo -n "freq" $FREQ " "
    $CMD_SF $FREQ
    ;;
  9)
    echo  -n "bookmark 9: itatia 610.000 HZ "
    FREQ=610000
    echo -n "freq" $FREQ " "
    $CMD_SF $FREQ
    ;;
  0)
    echo  -n "bookmark 0: 750.000 HZ "
    FREQ=750000
    echo -n "freq" $FREQ " "
    $CMD_SF $FREQ
    ;;

  i)
    echo -n "insert freq in Hertz "
    read FREQ
    echo -n "freq" $FREQ "Hz"
    $CMD_SF $FREQ
    ;;

  +)
    echo -n "BFO +100 Hz:    "
    BFO=$(expr $BFO + 100)
    echo -n "freq ${BFO}  Hz"
    $CMD_SBFO ${BFO}
    ;;

    -)
      echo -n "BFO -100 Hz:    "
      BFO=$(expr $BFO - 100)
      echo -n "freq ${BFO}  Hz"
      $CMD_SBFO ${BFO}
    ;;

    '[A') # up
      echo -n "MASTERCAL +1000 Hz:    "
      MASTERCAL=$(expr $MASTERCAL + 1000)
      echo -n "offset ${MASTERCAL}"
      $CMD_SMASTERCAL ${MASTERCAL}
      ;;
    '[B') #down
      echo -n "MASTERCAL -1000 Hz:    "
      MASTERCAL=$(expr $MASTERCAL - 1000)
      echo -n "offset ${MASTERCAL}"
      $CMD_SMASTERCAL ${MASTERCAL}
      ;;
    '[D') # left
      echo -n "freq - 1 KHz:   "
      FREQ=$(expr $FREQ - 1000)
      echo -n "freq" $FREQ  "Hz"
      $CMD_SF $FREQ
      ;;
    '[C') # right
      echo -n "freq + 1 KHz:   "
      FREQ=$(expr $FREQ + 1000)
      echo -n "freq" $FREQ  "Hz"
      $CMD_SF $FREQ
    ;;

  q)
    echo  "exiting"
    exit
    ;;


  *)
    echo -n "unknown"
    ;;
esac

done
