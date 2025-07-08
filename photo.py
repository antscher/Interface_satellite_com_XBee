import struct
from datetime import datetime
import os

# Folder where images will be saved
STATIC_FOLDER = os.path.join(os.path.dirname(__file__), 'static')

# Frame delimiters for image data
START_SEQ = b'\xDE\xAD\xBE\xEF'
END_SEQ = b'\xFE\xED\xFA\xCE'

abort_flag = False  # Used to abort image reception

def receive_photo(ser):
    """
    Receives a JPEG image from the ESP32 over serial.
    Looks for start and end sequences, extracts image, and saves it.
    Returns the filename if successful, or None if aborted.
    """
    global abort_flag
    abort_flag = False  # Reset abort flag at start
    buffer = bytearray()

    while True:
        if abort_flag:
            print("📛 Capture aborted by user.")
            return None
        if ser.in_waiting:
            data = ser.read(ser.in_waiting)
            buffer.extend(data)

            while True:
                # Look for start sequence
                start_idx = buffer.find(START_SEQ)
                if start_idx == -1 or len(buffer) < start_idx + 8:
                    break

                # Extract image size (4 bytes, big-endian)
                size_bytes = buffer[start_idx + 4 : start_idx + 8]
                img_size = struct.unpack('>I', size_bytes)[0]
                img_start = start_idx + 8
                img_end = img_start + img_size

                # Wait for full image and end sequence
                if len(buffer) < img_end + 4:
                    break

                # Check for correct end sequence
                if buffer[img_end:img_end+4] != END_SEQ:
                    print("❌ Invalid end sequence")
                    buffer = buffer[start_idx + 1:]
                    continue

                # Extract image bytes
                frame = buffer[img_start:img_end]
                buffer = buffer[img_end + 4:]

                # Generate unique filename using timestamp
                filename = f"image_{datetime.now().strftime('%Y%m%d_%H%M%S')}.jpg"
                full_path = os.path.join(STATIC_FOLDER, filename)
                with open(full_path, "wb") as f:
                    f.write(frame)
                print(f"[+] Saved {filename}")
                return filename

def abort_receive():
    """
    Sets the abort flag to stop image reception.
    """
    global abort_flag
    abort_flag = True