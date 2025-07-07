import struct
from datetime import datetime
import os

STATIC_FOLDER = os.path.join(os.path.dirname(__file__), 'static')

START_SEQ = b'\xDE\xAD\xBE\xEF'
END_SEQ = b'\xFE\xED\xFA\xCE'

def receive_photo(ser):
    buffer = bytearray()

    while True:
        if ser.in_waiting:
            data = ser.read(ser.in_waiting)
            buffer.extend(data)

            while True:
                start_idx = buffer.find(START_SEQ)
                if start_idx == -1 or len(buffer) < start_idx + 8:
                    break

                size_bytes = buffer[start_idx + 4 : start_idx + 8]
                img_size = struct.unpack('>I', size_bytes)[0]
                img_start = start_idx + 8
                img_end = img_start + img_size

                if len(buffer) < img_end + 4:
                    break

                if buffer[img_end:img_end+4] != END_SEQ:
                    print("❌ Invalid end sequence")
                    buffer = buffer[start_idx + 1:]
                    continue

                frame = buffer[img_start:img_end]
                buffer = buffer[img_end + 4:]

                # Nom unique par timestamp
                filename = f"image_{datetime.now().strftime('%Y%m%d_%H%M%S')}.jpg"
                full_path = os.path.join(STATIC_FOLDER, filename)
                with open(full_path, "wb") as f:
                    f.write(frame)
                print(f"[+] Saved {filename}")
                return full_path
