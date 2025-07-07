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
from photo import receive_photo

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
            ser.close()
        ser = serial.Serial(port, 115200, timeout=1)
        return jsonify({"success": True, "message": f"Connected to {port}"})
    except Exception as e:
        return jsonify({"success": False, "message": str(e)})

@app.route("/capture", methods=["GET"])
def capture():
    global ser
    if ser is None or not ser.is_open:
        return jsonify({"success": False, "message": "Not connected to any port."})

    try:
        ser.write(b'c')
        filename = receive_photo(ser)  # Should return path in static/
        return jsonify({"success": True, "filename": os.path.basename(filename)})
    except Exception as e:
        return jsonify({"success": False, "message": str(e)})

if __name__ == "__main__":
    app.run(debug=True)

