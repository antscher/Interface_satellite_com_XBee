// camera_sender.h
#ifndef CAMERA_SENDER_H
#define CAMERA_SENDER_H

#include <esp_camera.h>
#include <HardwareSerial.h>

// ==== UART2 definition for XBee ====
extern HardwareSerial XBeeSerial; // UART2 instance for XBee communication

// === Exposed functions ===

// Initialize camera and UART2 (XBee)
void init_camera_and_uart();

// Capture image and send via UART2 (XBee)
void take_and_send_picture();

#endif
