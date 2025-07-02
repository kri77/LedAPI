// This code is for an Arduino sketch that controls a set of LEDs by posting commands to turn them on and off, this also allows for serial communication to check the status of each LED.
// The state of each LED can be queried via the serial command STATUS. To set the state of a LED, provide a string with 0 and 1 corresponding to the LED pin numbers.
// Example commands: 1000 for red, 0100 for yellow, 0010 for green, and 0001 for blue. Multiple LEDs can be set at once by combining these values, e.g., 1100 for red and yellow on, or 1111 for all LEDs on.


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

// Serial input
String inputString = "";

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  Serial.begin(115200);

  // Start with all LEDs OFF
  setLED(redPin, redState, LOW);
  setLED(yellowPin, yellowState, LOW);
  setLED(greenPin, greenState, LOW);
  setLED(bluePin, blueState, LOW);
}

void loop() {
  handleSerial();
}

void handleSerial() {
  if (Serial.available()) {
    inputString = Serial.readStringUntil('\n');
    inputString.trim();

    if (inputString.equalsIgnoreCase("STATUS")) {
      // Report current LED states
      String status = "redPin:" + String(redPin) + "=" + String(redState == HIGH ? 1 : 0) + " ";
      status += "yellowPin:" + String(yellowPin) + "=" + String(yellowState == HIGH ? 1 : 0) + " ";
      status += "greenPin:" + String(greenPin) + "=" + String(greenState == HIGH ? 1 : 0) + " ";
      status += "bluePin:" + String(bluePin) + "=" + String(blueState == HIGH ? 1 : 0);
      Serial.println(status);
    }
    else if (inputString.length() == 4 &&
             (inputString.charAt(0) == '0' || inputString.charAt(0) == '1') &&
             (inputString.charAt(1) == '0' || inputString.charAt(1) == '1') &&
             (inputString.charAt(2) == '0' || inputString.charAt(2) == '1') &&
             (inputString.charAt(3) == '0' || inputString.charAt(3) == '1')) {

      // Convert each character to LED state
      setLED(redPin, redState, inputString.charAt(0) == '1' ? HIGH : LOW);
      setLED(yellowPin, yellowState, inputString.charAt(1) == '1' ? HIGH : LOW);
      setLED(greenPin, greenState, inputString.charAt(2) == '1' ? HIGH : LOW);
      setLED(bluePin, blueState, inputString.charAt(3) == '1' ? HIGH : LOW);

      Serial.println("LED state updated.");
    }
    else {
      Serial.println("Invalid command.");
    }

    inputString = "";
  }
}

//Pointers doing its thing with stateVar
void setLED(int pin, int &stateVar, int value) {
  digitalWrite(pin, value);
  stateVar = value;
}

