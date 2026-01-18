import serial #pyserial is required for serial communication with Arduino
import serial.tools.list_ports
import time
import os
from flask import Flask, request, jsonify #We use Flask to create a simple REST API
from flask_swagger_ui import get_swaggerui_blueprint
from flask_cors import CORS

#To be able to control the LEDs be sure to upload ControlLedsWithCommands.ino to your Arduino
#BlinkingLightsWithStatesReturned.ino only returns the LED statuses, but ignores the control commands

app = Flask(__name__)
CORS(app) #We need to enable CORS to allow requests 
SWAGGER_URL = '/docs'  # URL to access docs
API_URL = '/static/arduino_led_api.yaml'  # URL for the Swagger API specification file

swaggerui_blueprint = get_swaggerui_blueprint(
    SWAGGER_URL, 
    API_URL,
    config={'app_name': "Arduino LED Controller API"}
)

#set up the arduino communication
app.register_blueprint(swaggerui_blueprint, url_prefix=SWAGGER_URL)
BAUD_RATE = 115200  # Standard baud rate for Arduino communication
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

# Parse and validate LED command format
def parse_led_command(command_str):
    """
    Parse LED command in format: SSSS:OOOO:III or simple SSSS
    
    SSSS = 4-digit LED state (0=off, 1=on steady, 2=blink)
    OOOO = 4-digit blink sequence order (0=no sequence, 1-4=order position)
    III  = Blink interval in milliseconds (optional, default 500ms)
    
    Returns: dict with command details or None if invalid
    """
    command_str = command_str.strip().upper()
    
    # Handle STATUS command
    if command_str == "STATUS":
        return {"type": "status"}
    
    # Check for extended format (SSSS:OOOO:III or SSSS:OOOO)
    if ':' in command_str:
        parts = command_str.split(':')
        
        if len(parts) < 2 or len(parts) > 3:
            return None
        
        states = parts[0].strip()
        order = parts[1].strip()
        interval = int(parts[2].strip()) if len(parts) == 3 else 500
        
        # Validate states (0, 1, or 2)
        if len(states) != 4 or not all(c in "012" for c in states):
            return None
        
        # Validate order (0-4 for each position)
        if len(order) != 4 or not all(c in "01234" for c in order):
            return None
        
        # Validate interval 100 to 5000 ms
        if interval < 100 or interval > 5000:
            return None
        
        return {
            "type": "extended",
            "states": states,
            "order": order,
            "interval": interval
        }
    
    # Backward compatibility: simple 4-digit command (0s and 1s only)
    elif len(command_str) == 4 and all(c in "01" for c in command_str):
        return {
            "type": "simple",
            "states": command_str
        }
    
    return None


# Print LED pattern in terminal
def print_led_pattern(pattern, state_type=None):
    """Print LED status as colored rectangles in the terminal"""
    led_names = ['RED', 'YELLOW', 'GREEN', 'BLUE']
    led_colors = [
        '\033[41m  \033[0m',  # Red
        '\033[43m  \033[0m',  # Yellow
        '\033[42m  \033[0m',  # Green
        '\033[44m  \033[0m'   # Blue
    ]
    
    state_labels = {
        '0': 'OFF',
        '1': 'ON (Steady)',
        '2': 'BLINK'
    }
    
    print("\nLED Status:")
    for i, (bit, name, color) in enumerate(zip(pattern, led_names, led_colors)):
        if state_type:
            label = state_labels.get(bit, 'UNKNOWN')
        else:
            label = 'ON' if bit == '1' else 'OFF'
        
        if bit == '0':
            print(f"{name}: \033[40m  \033[0m {label}")
        else:
            print(f"{name}: {color} {label}")
    print()

# Endpoint to set LED status, supports multiple formats
@app.route('/setLedStatus', methods=['POST'])
def set_led_status():
    """
    Set LED status with support for multiple command formats:
    - Simple: "1010" (backward compatible)
    - Extended: "2020:0000:1000" (state:sequence:interval)
    - Extended: "2222:1234:250" (state:sequence with custom interval)
    """
    global arduino
    if arduino is None:
        return jsonify({"error": "Arduino not connected"}), 500

    data = request.get_json()
    command_input = data.get("pattern", "").strip()

    # Parse the command
    parsed = parse_led_command(command_input)
    
    if parsed is None:
        return jsonify({
            "error": "Invalid command format. Use one of:\n"
                     "- Simple: '1010' (backward compatible)\n"
                     "- Extended: 'SSSS:OOOO:III' (state:sequence:interval)\n"
                     "- STATUS (query LED states)"
        }), 400

    try:
        if parsed["type"] == "status":
            # Status query handled separately
            return jsonify({"message": "Use GET /status endpoint to query LED states"}), 400
        
        # For simple format, send as-is (backward compatible)
        if parsed["type"] == "simple":
            print_led_pattern(parsed["states"])
            arduino.write(f"{parsed['states']}\n".encode())
            return jsonify({
                "status": f"Sent simple pattern '{parsed['states']}' to Arduino",
                "command": parsed['states']
            }), 200
        
        # For extended format, construct command with all parameters
        if parsed["type"] == "extended":
            states = parsed["states"]
            order = parsed["order"]
            interval = parsed["interval"]
            
            # Print enhanced information
            print_led_pattern(states, state_type="extended")
            print(f"Sequence Order: {order}")
            print(f"Blink Interval: {interval}ms")
            
            # Send command in extended format to Arduino
            # Format: SSSS:OOOO:III
            command = f"{states}:{order}:{interval}\n"
            arduino.write(command.encode())
            
            return jsonify({
                "status": "Sent extended command to Arduino",
                "command": command.strip(),
                "details": {
                    "states": states,
                    "sequence_order": order,
                    "interval_ms": interval
                }
            }), 200
    
    except Exception as e:
        return jsonify({"error": f"Failed to send command: {str(e)}"}), 500

# Endpoint to check Arduino connection status
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
# How to use the API:
#
# Endpoint: POST /setLedStatus
# Content-Type: application/json
#
# Provide a pattern using 0 1 and 2 to set led states off, on and blink.
#
# Supported command formats:
# 1. Simple format (backward compatible):
#    Body: { "pattern": "1010" }
#    LEDs 1 and 3 ON steady
#
# 2. Use this to enable a blink sequence:
#    Body: { "pattern": "2020:0000:1000" }
#    LEDs 1 and 3 blink simultaneously at 1000ms
#
# 3. Use this to enable a blink sequence with order:
#    Body: { "pattern": "2222:1234:250" }
#    All LEDs blink in sequence at 250ms intervals
#
# 4. Use this to set different states and sequence order:
#    Body: { "pattern": "1210:0021:500" }
#    → LED 1 steady, LED 2 blinks, LED 4 blinks (sequence order 2 1)
#
# Status query:
# Endpoint: GET /status
# ================================