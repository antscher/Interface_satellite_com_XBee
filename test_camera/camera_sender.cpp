// camera_sender.cpp
#include "camera_sender.h"

// ==== OV2640 camera configuration ====
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      21
#define SIOD_GPIO_NUM      26
#define SIOC_GPIO_NUM      27

#define Y9_GPIO_NUM        35
#define Y8_GPIO_NUM        34
#define Y7_GPIO_NUM        39
#define Y6_GPIO_NUM        36
#define Y5_GPIO_NUM        19
#define Y4_GPIO_NUM        18
#define Y3_GPIO_NUM         5
#define Y2_GPIO_NUM         4
#define VSYNC_GPIO_NUM     25
#define HREF_GPIO_NUM      23
#define PCLK_GPIO_NUM      22

#define RXD2 32
#define TXD2 33

HardwareSerial XBeeSerial(2);  // UART2 for XBee

void init_camera_and_uart() {
  Serial.begin(115200);
  delay(1000);
  XBeeSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("✅ UART2 (XBee) initialized");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_XGA;  // 1024x768
  config.jpeg_quality = 5;            // Best quality
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("❌ Camera initialization error");
    return;
  }

  Serial.println("📷 Camera initialized");
}

void take_and_send_picture() {
  Serial.println("📸 Capturing image...");
  // Refresh the buffer
  camera_fb_t *fb = esp_camera_fb_get();
  if (fb) esp_camera_fb_return(fb);

  // take a new image
  fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("❌ Capture failed");
    XBeeSerial.println("❌ Capture failed");
    return;
  }

  Serial.printf("📦 Image: %u bytes\n", fb->len);

  // Start of frame
  XBeeSerial.write(0xDE);
  XBeeSerial.write(0xAD);
  XBeeSerial.write(0xBE);
  XBeeSerial.write(0xEF);

  // Size on 4 bytes (MSB first)
  XBeeSerial.write((fb->len >> 24) & 0xFF);
  XBeeSerial.write((fb->len >> 16) & 0xFF);
  XBeeSerial.write((fb->len >> 8) & 0xFF);
  XBeeSerial.write(fb->len & 0xFF);

  // JPEG data
  XBeeSerial.write(fb->buf, fb->len);

  // End of frame
  XBeeSerial.write(0xFE);
  XBeeSerial.write(0xED);
  XBeeSerial.write(0xFA);
  XBeeSerial.write(0xCE);

  esp_camera_fb_return(fb);

  Serial.println("✅ Image sent via XBee\n");
}


