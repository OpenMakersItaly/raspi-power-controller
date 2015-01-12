#!/bin/bash

# Original version taken from http://lowpowerlab.com/atxraspi/
# Adapted by Salvatore Carotenuto of StartupSolutions / OpenMakersItaly

# This script puts "BOOT" pin as high when it starts,
# and checks continously the status of "SHUTDOWN" pin. When this pin is put as
# high by the external controller circuit, issues a shutdown.

# GPIO26 (pin 37 on the pinout diagram of the RaspberryPi B+).
# This pin is used as input by the Raspberry. 
# When the system is running and the user presses the power button,
# this pin is set high by the ATTiny13, signaling the RaspberryPi to shutdown.
SHUTDOWN=26
echo "$SHUTDOWN" > /sys/class/gpio/export
echo "in" > /sys/class/gpio/gpio$SHUTDOWN/direction

# GPIO20 (pin 38 on the pinout diagram).
# This pin is used as input by the Raspberry. 
# This pin is put HIGH by the RaspberryPi when the system has booted.
BOOT=20
echo "$BOOT" > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio$BOOT/direction
echo "1" > /sys/class/gpio/gpio$BOOT/value

echo "RaspiPowerController shutdown script started: asserted pins ($SHUTDOWN=input,LOW; $BOOT=output,HIGH). Waiting for GPIO$SHUTDOWN to become HIGH..."

# This loop continuously checks if the shutdown button was pressed on RaspiPowerController (SHUTDOWN pin to become HIGH), and issues a shutdown when that happens.
# It sleeps as long as that has not happened.
while [ 1 ]; do
  shutdownSignal=$(cat /sys/class/gpio/gpio$SHUTDOWN/value)
  if [ $shutdownSignal = 0 ]; then
    /bin/sleep 0.5
  else
    sudo pkill xbmc.bin
    /bin/sleep 5
    sudo poweroff
  fi
done

