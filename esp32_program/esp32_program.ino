#include "camera_sender.h"
#include "sensors.h"
#include <Wire.h>

unsigned long lastSensorTime = 0;
const unsigned long interval = 10000; // 30 sec

void setup() {
  Serial.begin(115200);
  init_camera_and_uart();
  Serial.println("start");  
  Wire.begin(13,15);
  Serial.println("wire congigured");
  sensor_init_bme();
  Serial.println("BME configured");
  init_mpu();
  Serial.println("MPU configured");

}

void loop() {
  // Check if a command is available from XBee
  if (XBeeSerial.available()) {
    char cmd = XBeeSerial.read();
    if (cmd == 'c') {
      take_and_send_picture(); // Capture and send image if 'c' command received
    }
  }
  if (millis() - lastSensorTime > interval) {
    transmit_data(XBeeSerial);
    lastSensorTime = millis();
  }

  delay(100);  // Small pause
}
