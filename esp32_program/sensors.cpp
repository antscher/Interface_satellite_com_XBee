#include "Sensors.h"
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>

// BME280 (addr 0x76) 
Adafruit_BME280 bme;

// MPU6050 (addr 0x68) 
Adafruit_MPU6050 mpu;

bool sensor_init_bme() {
  return bme.begin(0x76, &Wire);
}

bool init_mpu() {
  return mpu.begin(0x68, &Wire);
}

void get_measure_bme(float &temperature, float &pressure, float &humidity) {
  temperature = bme.readTemperature();
  Serial.print("Temp: "); Serial.print(temperature); Serial.println(" °C");

  pressure = bme.readPressure() / 100.0F;
  Serial.print("Pressure: "); Serial.print(pressure); Serial.println(" hPa");

  humidity = bme.readHumidity();
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
}

void get_mpu(float &gx, float &gy, float &gz) {
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  gx = g.gyro.x;
  gy = g.gyro.y;
  gz = g.gyro.z;

  Serial.print("Gyro X: "); Serial.println(gx);
  Serial.print("Gyro Y: "); Serial.println(gy);
  Serial.print("Gyro Z: "); Serial.println(gz);
}

void get_accel(float &ax, float &ay, float &az) {
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  ax = a.acceleration.x;
  ay = a.acceleration.y;
  az = a.acceleration.z;

  Serial.print("Accel X: "); Serial.println(ax);
  Serial.print("Accel Y: "); Serial.println(ay);
  Serial.print("Accel Z: "); Serial.println(az);
}

void transmit_data(HardwareSerial &serial) {
  // En-tête
  serial.write(0xA1); serial.write(0xB2); serial.write(0xC3); serial.write(0xD4);

  // === BME280 === (ID = 2)
  float t, p, h;
  get_measure_bme(t, p, h);
  serial.write(0x00); serial.write(0x00); serial.write(0x00); serial.write(0x02); // ID BME
  serial.write((uint8_t *)&t, sizeof(float));
  serial.write((uint8_t *)&p, sizeof(float));
  serial.write((uint8_t *)&h, sizeof(float));

  // === Gyroscope MPU === (ID = 3)
  float gx, gy, gz;
  get_mpu(gx, gy, gz);
  serial.write(0x00); serial.write(0x00); serial.write(0x00); serial.write(0x03); // ID Gyro
  serial.write((uint8_t *)&gx, sizeof(float));
  serial.write((uint8_t *)&gy, sizeof(float));
  serial.write((uint8_t *)&gz, sizeof(float));

  // === Accéléromètre MPU === (ID = 4)
  float ax, ay, az;
  get_accel(ax, ay, az);
  serial.write(0x00); serial.write(0x00); serial.write(0x00); serial.write(0x04); // ID Accel
  serial.write((uint8_t *)&ax, sizeof(float));
  serial.write((uint8_t *)&ay, sizeof(float));
  serial.write((uint8_t *)&az, sizeof(float));

  // Fin de trame
  serial.write(0x1E); serial.write(0x2D); serial.write(0x3C); serial.write(0x4B);
}
