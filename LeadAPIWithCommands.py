import serial #pyserial is required for serial communication with Arduino
import serial.tools.list_ports
import subprocess
import sys
import time
import platform
import os
from flask import Flask, request, jsonify
from flask_swagger_ui import get_swaggerui_blueprint

app = Flask(__name__)
SWAGGER_URL = '/docs'  # URL to access docs
API_URL = '/static/arduino_led_api.yaml'  # Location of your OAS YAML

swaggerui_blueprint = get_swaggerui_blueprint(
    SWAGGER_URL, 
    API_URL,
    config={'app_name': "Arduino LED Controller API"}
)

app.register_blueprint(swaggerui_blueprint, url_prefix=SWAGGER_URL)
BAUD_RATE = 115200 
arduino = None
SERIAL_PORT = None


def find_arduino_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if ("Arduino" in port.description
                or "CH340" in port.description
                or "USB Serial" in port.description
                or "FTDI" in port.description):
            return port.device
    return None


def connect_to_arduino():
    global arduino, SERIAL_PORT
    SERIAL_PORT = find_arduino_port()
    if SERIAL_PORT is None:
        print("Could not find Arduino.")
        return

    try:
        arduino = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)
        print(f"Connected to Arduino on {SERIAL_PORT}")
    except Exception as e:
        print(f"Failed to open serial port {SERIAL_PORT}: {e}")
        arduino = None


@app.route('/setLedStatus', methods=['POST'])
def set_led_status():
    global arduino
    if arduino is None:
        return jsonify({"error": "Arduino not connected"}), 500

    data = request.get_json()
    led_command = data.get("pattern", "").strip()

    if len(led_command) != 4 or not all(c in "01" for c in led_command):
        return jsonify({"error": "Invalid pattern. Use a 4-character string of 0s and 1s like '1010'."}), 400

    try:
        arduino.write(f"{led_command}\n".encode())
        return jsonify({"status": f"Sent pattern '{led_command}' to Arduino"}), 200
    except Exception as e:
        return jsonify({"error": f"Failed to send command: {str(e)}"}), 500


@app.route('/status', methods=['GET'])
def status():
    if SERIAL_PORT is None:
        return jsonify({"status": "Arduino not found", "port": None}), 404
    elif arduino is not None and arduino.is_open:
        arduino.reset_input_buffer()
        arduino.write(b'STATUS\n')
        time.sleep(0.1)  # Small delay to wait for the Arduino to cycle through its status
        reply = arduino.readline().decode().strip()
        return jsonify({"status": "connected", "port": SERIAL_PORT, "arduino_reply": reply}), 200
    else:        
        return jsonify({
            "status": "error",
            "port": SERIAL_PORT,
            "error": "Could not open serial port"
        }), 500

def main():
    # Only run the connection in the actual Flask app, not in the reloader, this prevents multiple connections 
    if os.environ.get("WERKZEUG_RUN_MAIN") == "true":
        connect_to_arduino()

    app.run(debug=True)

if __name__ == '__main__':
    main() 

# ================================
# How to use : 
#
# Endpoint: POST /setLedStatus
# Content-Type: application/json
# Body:
# {
#     "pattern": "1010"
# }
#
# This will turn ON the red and green LEDs, and OFF yellow and blue.
# ================================