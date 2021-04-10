SERIAL = /dev/ttyACM0
FQBN = arduino:avr:uno

compile:
	arduino-cli compile -p ${SERIAL} --fqbn ${FQBN} fm-radio
upload:
	arduino-cli upload -p ${SERIAL} --fqbn ${FQBN} fm-radio
monitor:
	minicom -b 57600 -D ${SERIAL}
