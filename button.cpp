#include "button.h"
#include "Arduino.h"

TButton::TButton(unsigned char p)
{
  pin = p;
  pinMode(pin, INPUT_PULLUP);
  previousLevel = peek();
}

TButton::~TButton()
{
}

bool TButton::peek()
{
  return !digitalRead(pin);
}

void TButton::loop()
{
  newLevel = peek();

  if (newLevel && !previousLevel)
  {
    posEdge = true;     //wordt alleen bij isPosEdge() weer op false gezet
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
