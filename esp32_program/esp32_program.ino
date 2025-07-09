#include "camera_sender.h"
#include "sensors.h"
#include <Wire.h>

uint8_t ID_satelite = 0x13 ; // Unique identifier for the satellite (XBee) (in this case 13)
uint8_t OBC = 0xAA; // On-Board Computer identifier (can be any byte, here using 0xAA)
uint8_t BO = 0xB0 ;// Consign for takink a picture
uint8_t GS = 0x01 ; // Ground Station identifier (can be any byte, here using 0x01)

uint8_t TAKE_PICT[] = { ID_satelite, OBC, BO, GS };

// Timing for periodic sensor transmission
unsigned long lastSensorTime = 0;
const unsigned long interval = 15000; // 15 sec

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
    uint8_t buffer[4];
    XBeeSerial.readBytes(buffer, 4);
    if (compareArrays(buffer, TAKE_PICT, 4)) {
      take_and_send_picture();
    }
    else {
       Serial.print("Commande inconnue : ");
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
  if (millis() - lastSensorTime > interval) {
    transmit_data(XBeeSerial);
    lastSensorTime = millis();
  }

  delay(100);  // Small pause to avoid busy loop
}

bool compareArrays(uint8_t *a, uint8_t *b, int length) {
  for (int i = 0; i < length; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}