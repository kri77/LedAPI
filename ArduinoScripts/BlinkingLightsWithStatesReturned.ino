// Non-blocking delays, returning state correctly with the status command 
// This code is for an Arduino sketch that controls a set of LEDs with non-blocking delays and allows for serial communication to check the status of each LED.
// The LEDs will blink in a sequence, and the state of each LED can be queried via serial command STATUS.

// Pin assignments
const int redPin = 12;
const int yellowPin = 11;
const int greenPin = 10;
const int bluePin = 9;

// LED states
int redState = LOW;
int yellowState = LOW;
int greenState = LOW;
int blueState = LOW;

// Timing control
unsigned long previousMillis = 0;
const long stepInterval = 100; // Time between steps in ms
int blinkStep = 0;

// Serial input
String inputString = "";

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  unsigned long currentMillis = millis();

  // Non-blocking LED sequence
  if (currentMillis - previousMillis >= stepInterval) {
    previousMillis = currentMillis;
    updateBlinkSequence();
  }

  // Listening for serial commands
  handleSerial();
}

void handleSerial() {
  if (Serial.available()) {
    inputString = Serial.readStringUntil('\n');
    inputString.trim();

    if (inputString == "STATUS") {
      String status = 
        "redPin:" + String(redPin) + "=" + String(redState == HIGH ? 1 : 0) + " " +
        "yellowPin:" + String(yellowPin) + "=" + String(yellowState == HIGH ? 1 : 0) + " " +
        "greenPin:" + String(greenPin) + "=" + String(greenState == HIGH ? 1 : 0) + " " +
        "bluePin:" + String(bluePin) + "=" + String(blueState == HIGH ? 1 : 0);
      Serial.println(status);
    }

    inputString = "";
  }
}

void updateBlinkSequence() {
  // Reset all LEDs to OFF first
  setLED(redPin, redState, LOW);
  setLED(yellowPin, yellowState, LOW);
  setLED(greenPin, greenState, LOW);
  setLED(bluePin, blueState, LOW);

  // Then start blinking sequence, one at a time, then all 
  switch (blinkStep) {
    case 0: setLED(redPin, redState, HIGH); break;
    case 1: setLED(yellowPin, yellowState, HIGH); break;
    case 2: setLED(greenPin, greenState, HIGH); break;
    case 3: setLED(bluePin, blueState, HIGH); break;
    case 4:
      setLED(redPin, redState, HIGH);
      setLED(yellowPin, yellowState, HIGH);
      setLED(greenPin, greenState, HIGH);
      setLED(bluePin, blueState, HIGH);
      break;
  }

  // Move to next step
  blinkStep = (blinkStep + 1) % 6; // Loop back after step 5 when mod(6) =0
}

//Pointers doing its thing with stateVar
void setLED(int pin, int &stateVar, int value) {
  digitalWrite(pin, value);
  stateVar = value;
}
