#include "camera_sender.h"

void setup() {
  init_camera_and_uart();
}

void loop() {
  // Check if a command is available from XBee
  if (XBeeSerial.available()) {
    char cmd = XBeeSerial.read();
    if (cmd == 'c') {
      take_and_send_picture(); // Capture and send image if 'c' command received
    }
  }

  delay(100);  // Small pause
}
