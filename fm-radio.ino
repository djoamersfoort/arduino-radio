#include <Wire.h>

#include <limits.h>

#include <radio.h>
#include <RDA5807M.h>
#include <SI4703.h>
#include <SI4705.h>
#include <TEA5767.h>

#include <RDSParser.h>
#include <LiquidCrystal.h>

#include "ClickEncoder.h"
#include <TimerOne.h>

// For lcd
const int rs = 12, en = 11, d4 = 6, d5 = 5, d6 = 4, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Volume icon
byte vol_icon[8] = {
  B00000, //
  B01010, // # #
  B10101, //# # #
  B01010, // # #
  B10101, //# # #
  B01010, // # #
  B00000, //
};

SI4703 radio;    // Create instance of SI4703
RDSParser rds;   // RDS Parser

// ClickEncoder
ClickEncoder *encoder;
int16_t encoder_last, encoder_value = 8750;

void timerIsr() { encoder->service(); }

// Pins
const int FREQ_BUTTON_PIN = 13;

const int BTN_VOL_UP_PIN = 9;
const int BTN_VOL_DOWN_PIN = 8;

const int BTN_MUTE = 10;
const int MUTE_BTN_LED = A0;
unsigned long timestamp = ULONG_MAX;

static RADIO_FREQ lastf = -1;
static unsigned long nextFreqTime = 0;
RADIO_FREQ f = 0;

// LCD graphics
void DisplayFrequency(RADIO_FREQ f) {
	char s[12];
	radio.formatFrequency(s, sizeof(s));
	//Serial.print("FREQ: "); Serial.println(s);

	lcd.setCursor(0, 1);
	lcd.print("FREQ:");
	lcd.print(s);
}
void DisplayServiceName(char *name) {
	Serial.print("RDS: "); Serial.println(name);

	lcd.setCursor(0, 0);
	lcd.print("RDS :");
	lcd.print(name);
}

void print_volume() {
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Volume: ");
	lcd.setCursor(0, 1);
	lcd.write(byte(0));
	lcd.setCursor(1, 1);
	for(int i = 0; i < radio.getVolume(); i++) {
		lcd.write(byte(0));
	}
	timestamp = millis() + 1000;
}

void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
	rds.processData(block1, block2, block3, block4);
}

void setup() {
	Serial.begin(57600);
	lcd.begin(0, 0);
	lcd.print("Radio...");
	delay(500);

	radio.init();
	radio.debugEnable();
	radio.setBandFrequency(RADIO_BAND_FM, 8750);
	radio.setMono(false);
	radio.setMute(false);
	radio.setVolume(8);

	// Set frequency
	encoder = new ClickEncoder(A2, A1, 10); // NOTE: 10 is empty
	encoder->setAccelerationEnabled(false);
	Timer1.initialize(1000);
	Timer1.attachInterrupt(timerIsr);
	encoder_last = -1;

	// RDS
	radio.attachReceiveRDS(RDS_process);
	rds.attachServicenNameCallback(DisplayServiceName);

	// LCD
	lcd.createChar(0, vol_icon);
	lcd.begin(16, 2);
	lcd.print("RDS :");
	lcd.setCursor(0, 1);
	lcd.print("FREQ:");

	pinMode(FREQ_BUTTON_PIN, INPUT_PULLUP);

	pinMode(BTN_VOL_UP_PIN, INPUT_PULLUP);
	pinMode(BTN_VOL_DOWN_PIN, INPUT_PULLUP);

	pinMode(BTN_MUTE, INPUT_PULLUP);
	pinMode(MUTE_BTN_LED, OUTPUT);
}

void loop() {
	if(millis() >= timestamp) {
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("RDS:");
		DisplayFrequency(encoder_value);
		radio.checkRDS();
		timestamp = ULONG_MAX;
		Serial.println("millis() >= timestamp");
	}
	if(digitalRead(BTN_VOL_UP_PIN) == LOW && radio.getVolume() < 15) {
		while(digitalRead(BTN_VOL_UP_PIN) == LOW) {
			delay(10);
		}
		radio.setVolume(radio.getVolume() + 1);
		print_volume();
	}
	if(digitalRead(BTN_VOL_DOWN_PIN) == LOW && radio.getVolume() > 0) {
		while(digitalRead(BTN_VOL_DOWN_PIN) == LOW) {
			delay(10);
		}
		radio.setVolume(radio.getVolume() - 1);
		print_volume();
	}

	int mute_button_state = digitalRead(BTN_MUTE);
	if(mute_button_state == LOW) {
		delay(100);
		do {
			mute_button_state = digitalRead(BTN_MUTE);
			delay(10);
		} while(mute_button_state == LOW);
		bool mute_state = radio.getMute();
		if(mute_state == false) {
			radio.setMute(true);
			digitalWrite(MUTE_BTN_LED, HIGH);
		}
		if(mute_state == true) {
			radio.setMute(false);
			digitalWrite(MUTE_BTN_LED, LOW);
		}
	}

	if(digitalRead(FREQ_BUTTON_PIN) == LOW) {
		encoder_value += encoder->getValue() * 5;
	}
	if(digitalRead(FREQ_BUTTON_PIN) == HIGH) {
		encoder_value += encoder->getValue() * 50;
	}
	if(encoder_value < 8749) {
		lcd.setCursor(0, 1);
		lcd.print("Error: too low  ");
		delay(500);
		encoder_value = 8750;
		DisplayFrequency(encoder_value);
	}
	if(encoder_value > 10801) {
		lcd.setCursor(0, 1);
		lcd.print("Error: too high ");
		delay(500);
		encoder_value = 10800;
		DisplayFrequency(encoder_value);
	}

	if(encoder_value != encoder_last) {
		encoder_last = encoder_value;
		//Serial.print("Encoder value: ");
		//Serial.println(encoder_value);

		radio.setFrequency(encoder_value);

		lcd.setCursor(0, 0);
		lcd.print("RDS :          ");
		delay(100);
	}

	radio.checkRDS();

	// Update display
	unsigned long now = millis();

	if(now > nextFreqTime) {
		f = encoder_value;
		if(f != lastf) {
			DisplayFrequency(f);
			lastf = f;
		}
		nextFreqTime = now + 400;
	}
}

