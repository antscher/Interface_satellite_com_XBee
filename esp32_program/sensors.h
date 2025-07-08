#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

bool sensor_init_bme();
bool init_mpu();

void get_measure_bme(float &temperature, float &pressure, float &humidity);
void get_mpu(float &gx, float &gy, float &gz);
void get_accel(float &ax, float &ay, float &az);

void transmit_data(HardwareSerial &serial);

#endif
