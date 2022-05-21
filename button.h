#ifndef _BUTTON
#define _BUTTON

class TButton
{
  private:
    unsigned char pin;
    bool newLevel;
    bool previousLevel;
    bool posEdge;
    bool negEdge;

    void loop();
  public:
    TButton(unsigned char);
    ~TButton();
    bool peek();
    bool isPosEdge();
    bool isNegEdge();
};

#endif
