# Arduino Nano LED Wiring

This document describes the wiring setup for the Arduino Nano and four LEDs: red, yellow, green, and blue.

---

## Wiring Overview

Each LED is connected to a dedicated digital output pin on the Arduino Nano, with a **220Ω resistor** in series.
All cathodes are connected to a **common ground (GND)**.

---

## Arduino Pin Mapping

| LED Color | Arduino Pin | Resistor | GND Connection |
|-----------|-------------|----------|----------------|
| Red       | D12         | 220Ω     | Shared         |
| Yellow    | D11         | 220Ω     | Shared         |
| Green     | D10         | 220Ω     | Shared         |
| Blue      | D9          | 220Ω     | Shared         |

---


## Notes

- The **anode** (long leg) of each LED connects to the Arduino pin via a 220Ω resistor.
- The **cathode** (short leg) connects to a common ground rail that goes to Arduino GND.
- The Arduino Nano is powered via USB, and the GND pin serves as reference ground.

---

## Test Checklist

- LEDs light up in sequence as defined in the sketch.
- Each LED has a 220Ω resistor in series to limit current.
- All cathodes are correctly connected to a shared ground.

---


MIT License · Feel free to use, modify, and share.
