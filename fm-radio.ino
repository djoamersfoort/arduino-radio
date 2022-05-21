#include <Arduino.h>
#include <Wire.h>

#include <limits.h>

#include <radio.h>
//#include <RDA5807M.h>
#include <SI4703.h>
//#include <SI4705.h>
//#include <TEA5767.h>

#include <RDSParser.h>
#include <LiquidCrystal.h>

#include "Rotary.h"
#include "button.h"
#include "store.h"

const int EEPROM_FREQ_ADX = 0;

enum MYSTATES {
	STATE_INIT = 0,
	STATE_LISTEN,
	STATE_TUNE,
	STATE_VOLUP,
	STATE_VOLDWN,
	STATE_VOL_RELEASE,
	STATE_MUTE,
	STATE_MUTE_RELEASE,
	STATE_SLEEP,
	STATE_WAKE,
	STANDARD,
};
MYSTATES state = STATE_INIT;

void change_state(MYSTATES s) {
	Serial.print("State change: "); 
	Serial.print(state);

	state = s;

	Serial.print(" -> ");
	Serial.println(state);
}

// Pins
const unsigned char vol_up_pin   = 10;
const unsigned char vol_down_pin = 8;
const unsigned char mute_pin     = A0;
const unsigned char pwr_pin      = A3;
const unsigned char freq_pin     = 13;
// Buttons
TButton ButtonUp    (vol_up_pin);   // Volume up button
TButton ButtonDown  (vol_down_pin); // Volume down button
TButton ButtonMute  (mute_pin);     // Mute button
TButton ButtonPower (pwr_pin);      // Power button
// Radio
SI4703    radio;       // Create instance of SI4703
RDSParser rds;         // RDS Parser
char     *last_rds;    // RDS buffer
int       last_rds_freq;
// Rotary encoder
uint16_t encoder_value = read_eeprom(EEPROM_FREQ_ADX); // Used to keep track of frequency
bool     encoder_changed;      // Has the encoders position changed?
Rotary   r = Rotary(A2, A1);

//unsigned long timestamp = 0;
//RADIO_FREQ f = 0;

void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
	rds.processData(block1, block2, block3, block4);
}

// lcd
const int rs = 12, en = 11, d4 = 6, d5 = 5, d6 = 4, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Volume icon
byte icon[8] = {
	B00000, //
	B01010, //  # #
	B10101, // # # #
	B01010, //  # #
	B10101, // # # #
	B01010, //  # #
	B00000, //
};

// LCD graphics

// Display the frequency on the lcd
/*
//  RDS : {}
	FREQ: 87.50 Mhz
*/
void DisplayFrequency() {
	char s[12];
	radio.formatFrequency(s, sizeof(s));

	lcd.setCursor(0, 1);
	lcd.print("FREQ:");
	lcd.print(s);
	//Serial.print("FREQ: ");
	//Serial.println(s);

	Serial.print("DisplayFrequency() =  "); Serial.println(s);
}

// Display RDS on the lcd
/*
	RDS : {name}
//  FREQ: 87.50 Mhz
*/
void DisplayServiceName(char *name) {
	lcd.setCursor(0, 0);
	lcd.print("RDS :");
	lcd.print(name);

	last_rds = name;
	last_rds_freq = encoder_value;

	Serial.print("DisplayServiceName() ="); Serial.println(name);
}
// Check if the frequency is within 87.50 <-> 108.00 Mhz.
int check_frequency(int value) {
	if(value < 8749) {
		lcd.setCursor(0, 1);
		lcd.print("Error: Too low  ");
		Serial.println("Low freq");
		delay(500);
		value = 8750;
	}
	if(value > 10801) {
		lcd.setCursor(0, 1);
		lcd.print("Error: Too high ");
		Serial.println("High freq");
		delay(500);
		value = 10800;
	}
	return value;
}
// Display the volume on the lcd
/*
	Volume: {radio.getVolume()}
	*******
*/
void print_volume() {
	int v = radio.getVolume();
	lcd.clear();

	lcd.setCursor(0, 0);
	lcd.print("Volume: ");
	lcd.setCursor(8, 0);
	lcd.print(v);

	lcd.setCursor(0, 1);
	lcd.write(byte(0));
	lcd.setCursor(1, 1);

	for(int i = 0; i < v; i++) {
		lcd.write(byte(0));
	}

	delay(100);
}

// Called if the state is STATE_VOLUP
void volume_up() {
	int v = radio.getVolume();
	if(v < 15) {
		radio.setVolume(v + 1);
		print_volume();
	}
}

// Called if the state is STATE_VOLDWN
void volume_down() {
	int v = radio.getVolume();
	if(v > 0) {
		radio.setVolume(v - 1);
		print_volume();
	}
}

void wake() {
		Serial.println("Turning on...");
		radio.init();
		lcd.display();
		radio.setMute(0);
}
void sleep() {
		Serial.println("Turing off...");
		radio.setMute(1);
		lcd.noDisplay();
		radio.term();

}

void setup() {
	Serial.begin(57600);
	lcd.createChar(0, icon);

	change_state(STATE_INIT);

	// Loading screen
	lcd.begin(16, 2);
	lcd.print("Loading...");

	// Radio
	radio.init();
	radio.debugEnable();
	radio.setBandFrequency(RADIO_BAND_FM, encoder_value);
	radio.setMono(false);
	radio.setMute(false);
	radio.setVolume(8);

	// Rotary
	r.begin(false, true);

	// RDS
	radio.attachReceiveRDS(RDS_process);
	rds.attachServicenNameCallback(DisplayServiceName);

	// Pins
	pinMode(freq_pin, INPUT);

	// EEPROM
	Serial.print("encoder_value="); Serial.println(encoder_value);

	// LCD
	lcd.clear();

	lcd.setCursor(0, 0);
	lcd.print("RDS :          ");
	DisplayFrequency();

	init_timer_1(100);
}

void do_states(void) {
	switch (state) {
		case STATE_INIT:
			{
				change_state(STATE_LISTEN);
				break;
			}
		case STATE_MUTE:
			{
				if(ButtonMute.isPosEdge()) {
					radio.setMute(false);
					change_state(STATE_LISTEN);
					lcd.setCursor(15, 0);
					lcd.write(" ");
				}
				if(ButtonUp.peek() || ButtonDown.peek()) {
					lcd.setCursor(15, 2);
					lcd.print("^");
					delay(100);
					lcd.setCursor(15, 2);
					lcd.print(" ");
				}
				break;
			}
		case STATE_LISTEN:
			{
				if(ButtonMute.isPosEdge()) {
					radio.setMute(true);
					change_state(STATE_MUTE);
					break;
				}
				if(ButtonUp.isPosEdge()) {
					change_state(STATE_VOLUP);
					break;
				}
				if(ButtonDown.isPosEdge()) {
					change_state(STATE_VOLDWN);
					break;
				}
				break;
			}
		case STATE_VOLUP:
			{
				if(ButtonUp.peek()) {
					volume_up();
					delay(100);
				} else {
					change_state(STATE_VOL_RELEASE);
				}
				break;
			}
		case STATE_VOLDWN:
			{
				if(ButtonDown.peek()) {
					volume_down();
					delay(100);
				} else {
					change_state(STATE_VOL_RELEASE);
				}
				break;
			}
		case STATE_VOL_RELEASE:
			{
				delay(100);
				lcd.clear();
				DisplayFrequency();
				DisplayServiceName("");
				change_state(STATE_LISTEN);
				break;
			}
		case STATE_SLEEP:
			if(ButtonPower.isNegEdge()) {
				change_state(STATE_WAKE);
			//radio.setVolume(last_volume);
			//should_power_on = false;
			}
			break;
		case STATE_TUNE:
			break;
		case STATE_MUTE_RELEASE:
			break;
		case STATE_WAKE:
			radio.init();
			lcd.display();
			break;
		case STANDARD:
			{
				Serial.println("STANDARD");
				//change_state(STATE_LISTEN);
				break;
			}
	}
}

void loop() {
	if(encoder_changed) {
		encoder_value = check_frequency(encoder_value);
		radio.setFrequency(encoder_value);
		DisplayFrequency();

		if(encoder_value == last_rds_freq) {
			DisplayServiceName(last_rds);
			Serial.println("Used last_rds for RDS.");
		} else {
			lcd.setCursor(0, 0);
			lcd.print("RDS :          ");
		}
		save_to_eeprom(EEPROM_FREQ_ADX, encoder_value);
		encoder_changed = false;
	}

	do_states();

	// Display something on the lcd when muted.
	if(radio.getMute() == true) {
		lcd.setCursor(15, 0);
		lcd.write(byte(0));
	}
/*
	// Power button
	static bool should_power_on = true;
	int power_button_state = ButtonPower.peek();
	if(ButtonPower.isPosEdge()) {
		sleep();
		should_power_on = !should_power_on;
	}
	if(should_power_on && !power_button_state) {
		wake();
		should_power_on = !should_power_on;
	}
*/
	radio.checkRDS();
}

void init_timer_1(int Herz)
{
	cli();

	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;

	OCR1A = (F_CPU / 256 / Herz) - 1;

	TCCR1B &= ~(1 << WGM13);  // CTC mode
	TCCR1B |=  (1 << WGM12);  // CTC mode
	TCCR1B &= ~(1 << WGM11);  // CTC mode
	TCCR1B &= ~(1 << WGM10);  // CTC mode

	TCCR1B |=  (1 << CS12);   // 256 prescaler
	TCCR1B &= ~(1 << CS11);   // 256 prescaler
	TCCR1B &= ~(1 << CS10);   // 256 prescaler

	TIMSK1 |=  (1 << OCIE1A); // enable timer-1 compare-A interrupt
	TIMSK1 &= ~(1 << OCIE1B); // disable timer-1 compare-B interrupt
	TIMSK1 &= ~(1 << TOIE1);  // disable timer-1 Overflow Interrupt Enable

	sei();
}

ISR(TIMER1_COMPA_vect)      // timer-1 compare interrupt service routine
{
	unsigned char result = r.process();
	if (result != DIR_NONE) {          // Has the encoder changed?
		int b = digitalRead(freq_pin);
		if (result == DIR_CW) {
			if(b == LOW)
				encoder_value += 10;
			else
				encoder_value += 100;
		}
		if (result == DIR_CCW) {
			if(b == LOW)
				encoder_value -= 10;
			else
				encoder_value -= 100;
		}
		encoder_changed = true;
	}
}

