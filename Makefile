#SERIAL = /dev/ttyACM0
SERIAL = /dev/ttyUSB0
#FQBN = arduino:avr:uno
FQBN = arduino:avr:nano:cpu=atmega328old

compile:
	arduino-cli compile -p ${SERIAL} --fqbn ${FQBN} fm-radio --warnings "all"
upload:
	arduino-cli upload -p ${SERIAL} --fqbn ${FQBN} fm-radio
monitor:
	minicom -b 57600 -D ${SERIAL}
