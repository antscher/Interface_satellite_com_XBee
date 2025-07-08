#include "camera_sender.h"
#include "sensors.h"
#include <Wire.h>

// Timing for periodic sensor transmission
unsigned long lastSensorTime = 0;
const unsigned long interval = 10000; // 10 sec

void setup() {
  Serial.begin(115200);                // Debug serial
  init_camera_and_uart();              // Camera + UART2 (XBee) setup
  Serial.println("start");  
  Wire.begin(13, 15);                  // I2C bus for sensors (SDA=13, SCL=15)
  Serial.println("wire congigured");
  sensor_init_bme();                   // BME280 sensor init
  Serial.println("BME configured");
  init_mpu();                          // MPU6050 sensor init
  Serial.println("MPU configured");
}

void loop() {
  // --- Handle image capture command from XBee ---
  if (XBeeSerial.available()) {
    char cmd = XBeeSerial.read();
    if (cmd == 'c') {
      take_and_send_picture(); // Capture and send image if 'c' command received
    }
  }

  // --- Periodically send sensor data ---
  if (millis() - lastSensorTime > interval) {
    transmit_data(XBeeSerial);
    lastSensorTime = millis();
  }

  delay(100);  // Small pause to avoid busy loop
}
