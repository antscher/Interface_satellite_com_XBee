# Main Flask web server for the ESP32/XBee camera and sensor project

import time
from flask import Flask, render_template, request, jsonify
import serial
import threading
import os
from photo import receive_photo, abort_receive
import sensors
import logging

# Command bytes for taking a picture (protocol: [satellite][OBC][command][GS])
ID_satelite = b'\x13'  # Satellite identifier (example: 0x13)
OBC = b'\xAA'          # On-Board Computer identifier (example: 0xAA)
BO = b'\xB0'           # Command for uplink (take picture)
GS = b'\x01'           # Ground Station identifier (example: 0x01)
take_picture = ID_satelite + OBC + BO + GS  # 4-byte command to trigger image capture

# Uncomment to suppress Flask's default logging for cleaner output
# log = logging.getLogger('werkzeug')
# log.setLevel(logging.ERROR)

app = Flask(__name__)
ser = None  # Global serial object for XBee communication

@app.route("/")
def index():
    # Render the main web interface
    return render_template("index.html")

@app.route("/connect", methods=["POST"])
def connect():
    """
    Connect to the selected serial port (XBee).
    Starts the background thread for sensor data.
    """
    global ser
    data = request.get_json()
    port = data.get("port")
    try:
        # If already connected, stop previous thread and close port
        if ser and ser.is_open:
            sensors.running = False
            if sensors.serial_thread_obj:
                sensors.serial_thread_obj.join()
            ser.close()
        # Open new serial connection
        ser = serial.Serial(port, 115200, timeout=1)
        sensors.running = True
        # Start sensor reading thread
        sensors.serial_thread_obj = threading.Thread(target=sensors.serial_thread, args=(ser,), daemon=True)
        sensors.serial_thread_obj.start()
        return jsonify({"success": True, "message": f"Connected to {port}"})
    except Exception as e:
        return jsonify({"success": False, "message": str(e)})

@app.route("/uplink", methods=["POST"])
def uplink():
    """
    Send a custom 4-byte uplink command to the ESP32.
    The command should be a string like "0x13 0xAA 0xB0 0x01".
    """
    if not ser:
        return jsonify(success=False, error="Port série non connecté")

    try:
        data = request.get_json()
        cmd_str = data.get("command", "")
        if not cmd_str:
            return jsonify(success=False, error="Commande vide")

        # Convert string like "0x13 0xAA 0xB0 0x01" to bytes
        parts = cmd_str.strip().split()
        bytes_cmd = bytes(int(part, 16) for part in parts)

        # Pause sensor reading thread
        sensors.reading_paused = True
        time.sleep(0.5)

        ser.write(bytes_cmd)

        time.sleep(0.5)
        sensors.reading_paused = False

        return jsonify(success=True)

    except Exception as e:
        return jsonify(success=False, error=str(e))

@app.route("/capture")
def capture():
    """
    Trigger image capture on the ESP32.
    Pauses sensor reading thread to avoid data collision.
    """
    if ser:
        try:
            sensors.reading_paused = True  # Pause sensor thread
            time.sleep(0.5)  # Wait for pause to take effect
            ser.write(take_picture)  # Send 4-byte capture command to ESP32
            filename = receive_photo(ser)  # Receive and save image
            sensors.reading_paused = False  # Resume sensor thread
            if filename:
                return jsonify(success=True, filename=filename)
            else:
                return jsonify(success=False, error="Aborted")
        except Exception as e:
            return jsonify(success=False, error=str(e))
    return jsonify(success=False, error="Not connected")

@app.route("/abort")
def abort():
    """
    Abort an ongoing image reception.
    """
    abort_receive()
    sensors.reading_paused = False  # Resume sensor thread
    return jsonify(success=True)

@app.route("/sensor_data")
def sensor_data():
    """
    Return the latest sensor values as JSON.
    """
    return jsonify(sensors.latest_sensors)

@app.route("/disconnect", methods=["POST"])
def disconnect():
    """
    Disconnect from the serial port (XBee).
    Stops the background sensor thread and closes the port.
    """
    global ser
    try:
        if ser and ser.is_open:
            sensors.running = False
            if sensors.serial_thread_obj:
                sensors.serial_thread_obj.join()
            ser.close()
            ser = None
        return jsonify({"success": True, "message": "Disconnected"})
    except Exception as e:
        return jsonify({"success": False, "message": str(e)})

if __name__ == "__main__":
    # Start Flask development server
    app.run(debug=True)

