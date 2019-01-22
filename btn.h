#define debounce 50 
#define holdTime 1000 

class Btn {
    int buttonVal = 0; // value read from button
    int buttonLast = HIGH; // buffered value of the button's previous state
    long btnDnTime; // time the button was pressed down
    long btnUpTime; // time the button was released
    long lastHeld = 0;
    boolean ignoreUp = false; // whether to ignore the button release because the click+hold was triggered
    int buttonPin;

public:
    Btn(int pin) : buttonPin(pin) {
      pinMode(pin, INPUT_PULLUP);
    }

    bool pressed() {
      return buttonLast == LOW;
    }
    
    void poll(void (*pressed)(), void (*held)()) {
        // Read the state of the button
        buttonVal = digitalRead(buttonPin);

        // Test for button pressed and store the down time
        if (buttonVal == LOW && buttonLast == HIGH && (millis() - btnUpTime) > long(debounce)) {
          btnDnTime = millis();
        }

        // Test for button release and store the up time
        if (buttonVal == HIGH && buttonLast == LOW && (millis() - btnDnTime) > long(debounce)) {
          if (ignoreUp == false) {
            if (pressed) pressed();
          }
          else ignoreUp = false;
          btnUpTime = millis();
        }

        // Test for button held down for longer than the hold time
        if (buttonVal == LOW && (millis() - btnDnTime) > long(holdTime)) {
          if ((millis() - lastHeld) > 30) {
              if (held) held();
              lastHeld = millis();
          }
          ignoreUp = true;
        }

        buttonLast = buttonVal;
    }  
};
