// camera_sender.h
#ifndef CAMERA_SENDER_H
#define CAMERA_SENDER_H

#include "esp_camera.h"
#include <HardwareSerial.h>

// ==== UART2 definition for XBee ====
extern HardwareSerial XBeeSerial;

// === Exposed functions ===
void init_camera_and_uart();
void take_and_send_picture();

#endif
