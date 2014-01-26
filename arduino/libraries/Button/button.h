#ifndef OAC_BUTTON_H
#define OAC_BUTTON_H

#include "Arduino.h"

#define ON 1
#define OFF 0
#define DEFAULT_ON_STATE HIGH
#define DEFAULT_DEBOUNCE 200

typedef void (*NullaryCallback)();
typedef void (*UnaryCallback)(int state);

template <int OnState = DEFAULT_ON_STATE, 
          long Debounce = DEFAULT_DEBOUNCE>
class Button {
public:

  Button(int pin) : 
      _pin(pin), 
      _state(-1), 
      _timeMark(0), 
      _onPressed(0), 
      _onReleased(0),
      _onToggled(0) {
    pinMode(pin, INPUT);
  }

  int state() const { return _state; }
  
  void setOnPressed(NullaryCallback onPressed) { _onPressed = onPressed; }

  void setOnReleased(NullaryCallback onReleased) { _onReleased = onReleased; }

  void setOnToggled(UnaryCallback onToggled) { _onToggled = onToggled; }
  
  int check() {
    int prevState = _state;
    int newState = digitalRead(_pin);
    if ((newState != prevState) && (millis() - _timeMark > Debounce)) {
      _state = newState;
      if (_onPressed && (_state == OnState))
        _onPressed();
      else if (_onReleased && (_state != OnState))
        _onReleased();
      if (_onToggled)
        _onToggled(_state == OnState);
      _timeMark = millis();
    }
    return _state;
  }
  
private:

  int _pin;
  int _state;
  long _timeMark;
  NullaryCallback _onPressed;
  NullaryCallback _onReleased;
  UnaryCallback _onToggled;
};

template <int NumPos, 
          int OnState = DEFAULT_ON_STATE,
          long Debounce = DEFAULT_DEBOUNCE>
class RotarySwitch
{
public:

  typedef RotarySwitch<NumPos - 1, OnState, Debounce> ParentClass;

  RotarySwitch(int fromPin) : 
      _parent(fromPin),
      _pos(-1),
      _btn(fromPin + NumPos - 1) {
  }

  void setOnSelect(UnaryCallback onSelect) { _onSelect = onSelect; }

  int check() {
    int prevPos = _pos;
    int state = _btn.check();
    _pos = (state == OnState) ? (NumPos - 1) : _parent.check();
    if (_pos != -1 && _pos != prevPos && _onSelect) {
      _onSelect(_pos);
    }
    return _pos;
  }

private:

  ParentClass _parent;
  int _pos;
  Button<OnState, Debounce> _btn;
  UnaryCallback _onSelect;
};

template <int OnState,
          long Debounce>
class RotarySwitch<0, OnState, Debounce> {
public:
  
  RotarySwitch(int fromPin) {}

  void setOnSelect(UnaryCallback onSelect) {}

  int check() { return -1; }
};

#endif
