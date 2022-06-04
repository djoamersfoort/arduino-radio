#include "button.h"
#include "Arduino.h"

TButton::TButton(unsigned char p, bool m)
{
	pin = p;
	analog = m;
	if(analog) {
		pinMode(pin, INPUT);
	} else {
		pinMode(pin, INPUT_PULLUP);
	}
	previousLevel = peek();
}

TButton::~TButton()
{
}

bool TButton::peek()
{
	if(analog) {
		return analogRead(pin) > 950;
	} else {
		return !digitalRead(pin);
	}
}

void TButton::loop()
{
	newLevel = peek();

	if (newLevel && !previousLevel)
	{
		posEdge = true; //wordt alleen bij isPosEdge() weer op false gezet
	}

	if (!newLevel && previousLevel)
	{
		negEdge = true;
	}

	previousLevel = newLevel;
}

bool TButton::isPosEdge()
{
	loop();
	bool edge = posEdge;
	posEdge = false;
	negEdge = false;
	return edge;
}

bool TButton::isNegEdge()
{
	loop();
	bool edge = negEdge;
	posEdge = false;
	negEdge = false;
	return edge;
}
