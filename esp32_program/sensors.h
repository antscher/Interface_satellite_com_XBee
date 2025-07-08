#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// === Sensor initialization ===
bool sensor_init_bme();    // Initialize BME280 (temperature, pressure, humidity)
bool init_mpu();           // Initialize MPU6050 (gyro, accel)

// === Sensor reading functions ===
void get_measure_bme(float &temperature, float &pressure, float &humidity); // Read BME280
void get_mpu(float &gx, float &gy, float &gz);                              // Read gyroscope
void get_accel(float &ax, float &ay, float &az);                            // Read accelerometer

// === Data transmission ===
void transmit_data(HardwareSerial &serial); // Send all sensor data over UART

#endif
