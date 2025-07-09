#include "camera_sender.h"
#include "sensors.h"
#include <Wire.h>

// --- Command bytes definitions ---
uint8_t ID_satelite = 0x13;   // Unique identifier for the satellite (XBee)
uint8_t OBC = 0xAA;           // On-Board Computer identifier
uint8_t BO = 0xB0;            // Command for taking a picture
uint8_t REFRESH15 = 0x15;     // Command for 15s refresh interval
uint8_t REFRESH30 = 0x30;     // Command for 30s refresh interval
uint8_t REFRESH45 = 0x45;     // Command for 45s refresh interval
uint8_t REFRESH60 = 0x60;     // Command for 60s refresh interval
uint8_t NO_REFRESH = 0x00;    // Command for disabling periodic refresh
uint8_t GS = 0x01;            // Ground Station identifier

// --- Command arrays (4 bytes each) ---
uint8_t TAKE_PICT[]     = { ID_satelite, OBC, BO, GS };
uint8_t CMD_REFRESH15[] = { ID_satelite, OBC, REFRESH15, GS };
uint8_t CMD_REFRESH30[] = { ID_satelite, OBC, REFRESH30, GS };
uint8_t CMD_REFRESH45[] = { ID_satelite, OBC, REFRESH45, GS };
uint8_t CMD_REFRESH60[] = { ID_satelite, OBC, REFRESH60, GS };
uint8_t CMD_NO_REFRESH[] = { ID_satelite, OBC, NO_REFRESH, GS }; // No periodic sensor transmission

// --- Sensor transmission interval (default: 15 sec) ---
unsigned long lastSensorTime = 0;
unsigned long interval = 15000; // ms

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
  // --- Handle incoming 4-byte commands from XBee ---
  if (XBeeSerial.available() >= 4) {
    uint8_t buffer[4];
    XBeeSerial.readBytes(buffer, 4);

    // --- Take picture command ---
    if (compareArrays(buffer, TAKE_PICT, 4)) {
      take_and_send_picture();
    }
    // --- Change interval to 15s ---
    else if (compareArrays(buffer, CMD_REFRESH15, 4)) {
      interval = 15000;
      Serial.println("Interval changed to 15 seconds.");
    }
    // --- Change interval to 30s ---
    else if (compareArrays(buffer, CMD_REFRESH30, 4)) {
      interval = 30000;
      Serial.println("Interval changed to 30 seconds.");
    }
    // --- Change interval to 45s ---
    else if (compareArrays(buffer, CMD_REFRESH45, 4)) {
      interval = 45000;
      Serial.println("Interval changed to 45 seconds.");
    }
    // --- Change interval to 60s ---
    else if (compareArrays(buffer, CMD_REFRESH60, 4)) {
      interval = 60000;
      Serial.println("Interval changed to 60 seconds.");
    }
    // --- Disable periodic sensor transmission ---
    else if (compareArrays(buffer, CMD_NO_REFRESH, 4)) {
      interval = 0;
      Serial.println("Periodic sensor transmission disabled.");
    }
    // --- Unknown command ---
    else {
      Serial.print("Unknown command: ");
      for (int i = 0; i < 4; i++) {
        Serial.print("0x");
        if (buffer[i] < 0x10) Serial.print("0");
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
  }

  // --- Periodically send sensor data ---
  if (interval > 0 && millis() - lastSensorTime > interval) {
    transmit_data(XBeeSerial);
    lastSensorTime = millis();
  }

  delay(100);  // Small pause to avoid busy loop
}

// --- Utility: Compare two byte arrays ---
bool compareArrays(uint8_t *a, uint8_t *b, int length) {
  for (int i = 0; i < length; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}