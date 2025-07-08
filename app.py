# project structure:
# /project_root
#   |-- app.py
#   |-- photo.py
#   |-- static/
#   |     |-- style.css
#   |-- templates/
#         |-- index.html

# app.py
from flask import Flask, render_template, request, jsonify
import serial
import threading
import os
from photo import receive_photo,abort_receive
import sensors
import logging

log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)

app = Flask(__name__)
ser = None  # Global serial object

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/connect", methods=["POST"])
def connect():
    global ser
    data = request.get_json()
    port = data.get("port")
    try:
        if ser and ser.is_open:
            sensors.running = False
            if sensors.serial_thread_obj:
                sensors.serial_thread_obj.join()
            ser.close()
        ser = serial.Serial(port, 115200, timeout=1)
        sensors.running = True
        sensors.serial_thread_obj = threading.Thread(target=sensors.serial_thread, args=(ser,), daemon=True)
        sensors.serial_thread_obj.start()
        return jsonify({"success": True, "message": f"Connected to {port}"})
    except Exception as e:
        return jsonify({"success": False, "message": str(e)})

@app.route("/capture")
def capture():
    if ser:
        try:
            reading_paused = True  # pause thread
            ser.write(b'c')
            filename = receive_photo(ser)
            sensors.reading_paused = False  # reprend thread
            if filename:
                return jsonify(success=True, filename=filename)
            else:
                return jsonify(success=False, error="Aborted")
        except Exception as e:
            return jsonify(success=False, error=str(e))
    return jsonify(success=False, error="Not connected")

@app.route("/abort")
def abort():
    abort_receive()
    sensors.reading_paused = False # relance la lecture du thread
    return jsonify(success=True)

@app.route("/sensor_data")
def sensor_data():
    #print("Sending latest sensors:", sensors.latest_sensors)
    return jsonify(sensors.latest_sensors)

if __name__ == "__main__":
    app.run(debug=True)

