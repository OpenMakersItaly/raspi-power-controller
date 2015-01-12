
# bash script for flashing micro, with avrdude software and AVRISP MK-II programmer
# Author: Salvatore Carotenuto of Startup Solutions / OpenMakersItaly
#
# NOTE: change the value of "ISPPORT" with the last 4 digits of your AVRISP MK-II
# in-system programmer. You can find the serial number on the label on the back of the device.


MICRO="t13"
ISPMODEL="avrisp2"
ISPPORT="usb:84:76"
HEXIMAGE="./main.hex"

# internal RC Oscillator, 4.8MHz; Start-up time: 14 CK + 0 ms;
# CKDIV enabled
LFUSE="0x61"
HFUSE="0xFF"
LOCKB="0xFF"

# writes flash
avrdude -c $ISPMODEL -p $MICRO -P $ISPPORT -U flash:w:$HEXIMAGE -F -v

# writes fuses
avrdude -c $ISPMODEL -p $MICRO -P $ISPPORT -u -U hfuse:w:$HFUSE:m -U lfuse:w:$LFUSE:m -F

