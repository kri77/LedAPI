# Arduino LED API Controller

This project provides a simple **REST API** using Python + Flask that can control and monitor LEDs connected to an **Arduino Nano** via USB.  
The Arduino blinks LEDs in a fixed sequence, and the API allows querying the current LED states in real-time.  
This was a first attempt to read status from the Arduino using a local API and to command individual LEDS.

---

## Features

- Automatically detects connected Arduino Nano
- Sends commands to Arduino via USB serial
- `/status` endpoint to query current LED states (`0` or `1`)

---

## Requirements

- Python 3.7+
- Arduino Nano with uploaded sketch (see folder `ArduinoScripts`)
- USB connection between PC and Arduino
- `pyserial` and `flask` Python packages

---

## Installation

1. Clone or download this repository.
2. Install dependencies:

```bash
pip install -r requirements.txt
```

---

## API Usage
- See http://127.0.0.1:5000/docs/
- Get status by sending a GET to /status e.g. http://127.0.0.1:5000/status
- Set LED status by sending a POST to /setLedStatus e.g. http://127.0.0.1:5000/setLedStatus with parameters for the four leds as the body: {"pattern": "1011"}

---

## Python scripts

- `LeadApi.py` – For querying the status via the `STATUS` command only. Use this with `BlinkingLightsWithStatesReturned.ino` uploaded to the Arduino.
- `LeadApiWithCommands.py` – Enables API commands to turn individual LEDs ON or OFF. Also supports the STATUS command. Use this with `ControlLedsWithCommands.ino` uploaded to the Arduino

---

## Components for the circuit

| Component        | Quantity | Arduino Pin | Purpose          |
|------------------|----------|-------------|------------------|
| Arduino Nano     | 1        | —           | Main controller  |
| LED (Red)        | 1        | D12         | Visual indicator |
| LED (Yellow)     | 1        | D11         | Visual indicator |
| LED (Green)      | 1        | D10         | Visual indicator |
| LED (Blue)       | 1        | D9          | Visual indicator |
| Resistors (220Ω) | 4        | In series   | Current limiting |
| Breadboard       | 1        | —           | Prototyping      |
| Jumper wires     | 8        | —           | Connections      |

See also [View Wiring Instructions](wiring.md)
