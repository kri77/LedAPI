//This code is for an Arduino sketch that controls a set of LEDs with blocking delays , this was the first version, later version is non-blocking with actual state returned.

int redPin = 12;
int yellowPin = 11;
int greenPin = 10;
int bluePin = 9;

int redState = 0;
int yellowState = 0;
int greenState = 0;
int blueState = 0;

String inputString = "";

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  Serial.begin(115200);
}


//This version will always return state 0 because runLightSequence is not set in handleSerial, so we have an execution order problem.
void loop() {
  handleSerial();
  runLightSequence();
}

void handleSerial() {
  if (Serial.available()) {
    inputString = Serial.readStringUntil('\n');
    inputString.trim();

    if (inputString == "STATUS") {
      String status = "redPin:" + String(redPin) + "=" + String(redState) + " " +
                      "yellowPin:" + String(yellowPin) + "=" + String(yellowState) + " " +
                      "greenPin:" + String(greenPin) + "=" + String(greenState) + " " +
                      "bluePin:" + String(bluePin) + "=" + String(blueState);
      Serial.println(status);
    }

    inputString = "";
  }
}

void runLightSequence() {
  setLED(redPin, redState, HIGH); delay(100);
  setLED(redPin, redState, LOW);  delay(100);

  setLED(yellowPin, yellowState, HIGH); delay(100);
  setLED(yellowPin, yellowState, LOW);  delay(100);

  setLED(greenPin, greenState, HIGH); delay(100);
  setLED(greenPin, greenState, LOW);  delay(100);

  setLED(bluePin, blueState, HIGH); delay(100);
  setLED(bluePin, blueState, LOW);  delay(100);

  setLED(redPin, redState, HIGH);
  setLED(yellowPin, yellowState, HIGH);
  setLED(greenPin, greenState, HIGH);
  setLED(bluePin, blueState, HIGH);
  delay(100);

  setLED(redPin, redState, LOW);
  setLED(yellowPin, yellowState, LOW);
  setLED(greenPin, greenState, LOW);
  setLED(bluePin, blueState, LOW);
  delay(100);
}

void setLED(int pin, int &stateVar, int value) {
  digitalWrite(pin, value);
  stateVar = (value == HIGH) ? 1 : 0;
}
