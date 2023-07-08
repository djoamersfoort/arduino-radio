#SERIAL = /dev/ttyACM0
SERIAL = /dev/ttyUSB0
#FQBN = arduino:avr:uno
#FQBN = arduino:avr:nano:cpu=atmega328old
FQBN = arduino:avr:nano

compile:
	arduino-cli compile -p ${SERIAL} --fqbn ${FQBN} ../fm-radio --warnings "all"
upload:
	arduino-cli upload -p ${SERIAL} --fqbn ${FQBN} ../fm-radio --verbose
monitor:
	minicom -b 57600 -D ${SERIAL}
