#ifndef _BUTTON
#define _BUTTON

class TButton
{
  private:
    unsigned char pin;
	bool analog = false;
    bool newLevel;
    bool previousLevel;
    bool posEdge;
    bool negEdge;

    void loop();
  public:
	TButton(unsigned char, bool);
    ~TButton();
    bool peek();
    bool isPosEdge();
    bool isNegEdge();
};

#endif
