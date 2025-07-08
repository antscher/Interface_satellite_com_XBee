import time
import struct

# Thread and state variables for sensor reading
serial_thread_obj = None
running = False
reading_paused = False
latest_sensors = {}

# Frame delimiters for sensor data
HEADER = bytes([0xA1, 0xB2, 0xC3, 0xD4])
FOOTER = bytes([0x1E, 0x2D, 0x3C, 0x4B])

def parse_sensors(data):
    """
    Parse a block of sensor data into a dictionary.
    Each block: 4 bytes ID (big-endian) + 3 floats (little-endian).
    Returns: {sensor_id: (f1, f2, f3), ...}
    """
    sensors = {}
    offset = 0
    block_size = 4 + 3*4  # 4 bytes ID + 3 floats
    while offset + block_size <= len(data):
        sensor_id = struct.unpack(">I", data[offset:offset+4])[0]
        offset += 4
        f1, f2, f3 = struct.unpack("<fff", data[offset:offset+12])
        offset += 12
        sensors[sensor_id] = (f1, f2, f3)
    return sensors

def serial_thread(ser):
    """
    Background thread function to read and parse sensor data from serial.
    Updates latest_sensors dict with most recent values.
    """
    global running, latest_sensors
    buffer = bytearray()
    while running:
        if reading_paused:
            time.sleep(0.5)
            continue

        try:
            if ser.in_waiting:
                data = ser.read(ser.in_waiting)
                buffer.extend(data)
                print(f"Received {len(data)} bytes")

                while True:
                    # Look for header
                    start_idx = buffer.find(HEADER)
                    if start_idx == -1:
                        buffer.clear()
                        break
                    # Look for footer after header
                    end_idx = buffer.find(FOOTER, start_idx)
                    if end_idx == -1:
                        break

                    # Extract frame and parse sensors
                    frame_data = buffer[start_idx+len(HEADER):end_idx]
                    sensors = parse_sensors(frame_data)
                    print(f"Parsed sensors: {sensors}")

                    # Update latest_sensors dictionary with formatted values
                    if 2 in sensors:
                        t, p, h = sensors[2]
                        latest_sensors["temperature"] = f"{t:.2f}"
                        latest_sensors["pressure"] = f"{p:.2f}"
                        latest_sensors["humidity"] = f"{h:.2f}"
                    if 3 in sensors:
                        gx, gy, gz = sensors[3]
                        latest_sensors["gyro_x"] = f"{gx:.2f}"
                        latest_sensors["gyro_y"] = f"{gy:.2f}"
                        latest_sensors["gyro_z"] = f"{gz:.2f}"
                    if 4 in sensors:
                        ax, ay, az = sensors[4]
                        latest_sensors["acc_x"] = f"{ax:.2f}"
                        latest_sensors["acc_y"] = f"{ay:.2f}"
                        latest_sensors["acc_z"] = f"{az:.2f}"

                    # Remove processed frame from buffer
                    buffer = buffer[end_idx+len(FOOTER):]
            else:
                time.sleep(0.1)
        except Exception as e:
            print(f"Erreur lecture série: {e}")
            break
