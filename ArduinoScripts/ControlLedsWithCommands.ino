// This code controls 4 LEDs with support for steady on/off and blinking modes.
//
// COMMAND FORMAT: SSSS:OOOO:III
//   SSSS = 4-digit LED state (0=off, 1=on steady, 2=blink)
//   OOOO = 4-digit blink sequence order (0=no sequence, 1-4=order position)
//   III  = Blink interval in milliseconds (optional, default 500ms)
//
// BACKWARD COMPATIBLE: Simple 4-digit commands (e.g., "1010") work as before
//
// Examples:
//   1010              → LEDs 1 and 3 on steady
//   2020:0000:1000    → LEDs 1 and 3 blink simultaneously at 1000ms
//   2222:1234:250     → All LEDs blink in sequence at 250ms intervals
//   1210:0021:500     → LED 1 steady, LED 2 blinks, LED 4 blinks (sequence order 2 1)
//   STATUS            → Query current LED states


// Pin assignments
const int redPin = 12;
const int yellowPin = 11;
const int greenPin = 10;
const int bluePin = 9;

// LED state modes (0=off, 1=on steady, 2=blink)
int ledModes[4] = {0, 0, 0, 0};  // red, yellow, green, blue

// Blink sequence order (0=no sequence, 1-4=position in sequence)
int blinkOrder[4] = {0, 0, 0, 0};

// Current physical LED states
int ledStates[4] = {LOW, LOW, LOW, LOW};

// Blink timing
unsigned long blinkInterval = 500;  // Default 500ms
unsigned long previousMillis = 0;
int currentBlinkStep = 0;
bool blinkState = false;

// Serial input
String inputString = "";

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  Serial.begin(115200);

  // Start with all LEDs OFF
  updateAllLEDs();
}

void loop() {
  handleSerial();
  updateBlinking();
}

void handleSerial() {
  if (Serial.available()) {
    inputString = Serial.readStringUntil('\n');
    inputString.trim();

    if (inputString.equalsIgnoreCase("STATUS")) {
      printStatus();
    }
    else {
      // Parse the command
      if (parseCommand(inputString)) {
        Serial.println("LED state updated.");
        updateAllLEDs();
      }
    }

    inputString = "";
  }
}

// Parse command in format SSSS:OOOO:III or legacy SSSS
bool parseCommand(String cmd) {
  // Check for extended format with colons
  int firstColon = cmd.indexOf(':');
  int secondColon = cmd.lastIndexOf(':');

  String stateStr = "";
  String orderStr = "0000";
  String intervalStr = "500";

  if (firstColon == -1) {
    // Legacy format: just 4 digits (backward compatible)
    if (cmd.length() != 4) {
      Serial.println("ERROR: Legacy format requires exactly 4 digits. Example: 1010");
      return false;
    }
    stateStr = cmd;

    // Validate legacy format (only 0 and 1 allowed)
    for (int i = 0; i < 4; i++) {
      char c = stateStr.charAt(i);
      if (c != '0' && c != '1') {
        Serial.println("ERROR: Legacy format only accepts 0 (off) or 1 (on). Example: 1010");
        return false;
      }
    }
  }
  else {
    // Extended format: SSSS:OOOO:III
    if (firstColon != 4) {
      Serial.println("ERROR: State field must be 4 digits. Format: SSSS:OOOO:III");
      return false;
    }

    stateStr = cmd.substring(0, firstColon);

    if (secondColon > firstColon) {
      // Has interval field
      if (secondColon != 9) {
        Serial.println("ERROR: Order field must be 4 digits. Format: SSSS:OOOO:III");
        return false;
      }
      orderStr = cmd.substring(firstColon + 1, secondColon);
      intervalStr = cmd.substring(secondColon + 1);
    }
    else {
      // Only SSSS:OOOO
      orderStr = cmd.substring(firstColon + 1);
      if (orderStr.length() != 4) {
        Serial.println("ERROR: Order field must be 4 digits. Format: SSSS:OOOO or SSSS:OOOO:III");
        return false;
      }
    }

    // Validate state field (0, 1, or 2)
    if (stateStr.length() != 4) {
      Serial.println("ERROR: State field must be 4 digits. Format: SSSS:OOOO:III");
      return false;
    }
    for (int i = 0; i < 4; i++) {
      char c = stateStr.charAt(i);
      if (c != '0' && c != '1' && c != '2') {
        Serial.println("ERROR: State must be 0 (off), 1 (steady on), or 2 (blink). Format: SSSS:OOOO:III");
        return false;
      }
    }

    // Validate order field (0-4)
    if (orderStr.length() != 4) {
      Serial.println("ERROR: Order field must be 4 digits. Format: SSSS:OOOO:III");
      return false;
    }
    for (int i = 0; i < 4; i++) {
      char c = orderStr.charAt(i);
      if (c < '0' || c > '4') {
        Serial.println("ERROR: Order must be 0-4 (0=no sequence, 1-4=position). Format: SSSS:OOOO:III");
        return false;
      }
    }

    // Validate interval (must be a number)
    long interval = intervalStr.toInt();
    if (interval <= 0 || interval > 10000) {
      Serial.println("ERROR: Interval must be 1-10000 milliseconds. Format: SSSS:OOOO:III");
      return false;
    }
  }

  // Apply the parsed values
  for (int i = 0; i < 4; i++) {
    ledModes[i] = stateStr.charAt(i) - '0';
    blinkOrder[i] = orderStr.charAt(i) - '0';
  }

  blinkInterval = intervalStr.toInt();
  currentBlinkStep = 0;
  previousMillis = millis();

  return true;
}

// Print current status
void printStatus() {
  const char* modeNames[] = {"OFF", "ON", "BLINK"};

  Serial.print("Red(");
  Serial.print(redPin);
  Serial.print("): ");
  Serial.print(modeNames[ledModes[0]]);
  if (ledModes[0] == 2) {
    Serial.print(" [seq:");
    Serial.print(blinkOrder[0]);
    Serial.print("]");
  }

  Serial.print(" | Yellow(");
  Serial.print(yellowPin);
  Serial.print("): ");
  Serial.print(modeNames[ledModes[1]]);
  if (ledModes[1] == 2) {
    Serial.print(" [seq:");
    Serial.print(blinkOrder[1]);
    Serial.print("]");
  }

  Serial.print(" | Green(");
  Serial.print(greenPin);
  Serial.print("): ");
  Serial.print(modeNames[ledModes[2]]);
  if (ledModes[2] == 2) {
    Serial.print(" [seq:");
    Serial.print(blinkOrder[2]);
    Serial.print("]");
  }

  Serial.print(" | Blue(");
  Serial.print(bluePin);
  Serial.print("): ");
  Serial.print(modeNames[ledModes[3]]);
  if (ledModes[3] == 2) {
    Serial.print(" [seq:");
    Serial.print(blinkOrder[3]);
    Serial.print("]");
  }

  Serial.print(" | Interval: ");
  Serial.print(blinkInterval);
  Serial.println("ms");
}

// Update all LEDs based on their modes
void updateAllLEDs() {
  const int pins[] = {redPin, yellowPin, greenPin, bluePin};

  for (int i = 0; i < 4; i++) {
    if (ledModes[i] == 0) {
      // OFF
      digitalWrite(pins[i], LOW);
      ledStates[i] = LOW;
    }
    else if (ledModes[i] == 1) {
      // ON steady
      digitalWrite(pins[i], HIGH);
      ledStates[i] = HIGH;
    }
    // Mode 2 (BLINK) is handled by updateBlinking()
  }
}

// Handle blinking LEDs with non-blocking timing
void updateBlinking() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;

    // Check if any LED uses sequencing
    bool hasSequence = false;
    for (int i = 0; i < 4; i++) {
      if (ledModes[i] == 2 && blinkOrder[i] > 0) {
        hasSequence = true;
        break;
      }
    }

    if (hasSequence) {
      // Sequential blinking
      currentBlinkStep++;
      if (currentBlinkStep > 4) {
        currentBlinkStep = 1;
      }

      const int pins[] = {redPin, yellowPin, greenPin, bluePin};
      for (int i = 0; i < 4; i++) {
        if (ledModes[i] == 2) {
          if (blinkOrder[i] == currentBlinkStep) {
            digitalWrite(pins[i], HIGH);
            ledStates[i] = HIGH;
          }
          else {
            digitalWrite(pins[i], LOW);
            ledStates[i] = LOW;
          }
        }
      }
    }
    else {
      // Simultaneous blinking
      blinkState = !blinkState;

      const int pins[] = {redPin, yellowPin, greenPin, bluePin};
      for (int i = 0; i < 4; i++) {
        if (ledModes[i] == 2) {
          digitalWrite(pins[i], blinkState ? HIGH : LOW);
          ledStates[i] = blinkState ? HIGH : LOW;
        }
      }
    }
  }
}

