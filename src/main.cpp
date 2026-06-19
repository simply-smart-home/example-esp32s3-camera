/*
ESP32-S3 OV5640 WEB SERVER
By: Simply Smart X - Albert

DO NOT REDISTRIBUTE !
PERSONAL OR RESEARCH USE ONLY !

This Code Is Licensed Under: CC BY-NC-ND
This license enables reusers to copy and distribute the material in any medium or format in unadapted form only, 
for noncommercial purposes only, and only if attribution is given to the creator.

USB Connection Guide:
- Connect USB-OTG for programming
- Connect USB-UART for serial monitor and powering the board. Connecting to USB-OTG may cause camera unusable.

Power Consumption:
+- 0.15 A @ Camera Standby (XCLK off, no transmission)
+- 0.30 A @ Capture and Wi-Fi Transmission (XCLK on, capture, Wi-Fi transmission)
*/

#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// ==========================================================
// WIFI CONFIGURATION - CHANGE THESE TO YOUR WIFI CREDENTIALS
// ==========================================================
const char* ssid = "YOUR WIFI_SSID";
const char* password = "YOUR WIFI_PASSWORD";

// ==========================================================
// PIN CONFIGURATION
// ==========================================================
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  15
#define SIOD_GPIO_NUM  4
#define SIOC_GPIO_NUM  5

#define Y9_GPIO_NUM    16
#define Y8_GPIO_NUM    17
#define Y7_GPIO_NUM    18
#define Y6_GPIO_NUM    12
#define Y5_GPIO_NUM    10
#define Y4_GPIO_NUM    8
#define Y3_GPIO_NUM    9
#define Y2_GPIO_NUM    11
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM  7
#define PCLK_GPIO_NUM  13

// ==========================================================
// GLOBAL VARIABLES
// ==========================================================
WebServer server(80);
bool cameraAsleep = false;

// ==========================================================
// FUNCTION DECLARATIONS
// ==========================================================
void initCamera();

void setCameraStandby(bool enable) {
  if (cameraAsleep == enable) return; 
  if (enable) {
    //SLEEP PROCESS
    esp_camera_deinit();
    pinMode(XCLK_GPIO_NUM, OUTPUT);
    digitalWrite(XCLK_GPIO_NUM, LOW);
    Serial.println("Camera driver de-initialized !");
  } else {
    //WAKE UP PROCESS
    Serial.println("Camera waking up (re-initializing)...");
    initCamera();
    delay(500);
  }
  cameraAsleep = enable;
}

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;

  if (psramFound()) { 
    // For ESP32S3 sold by Smart Electronic Stores have PSRAM 
    config.frame_size = FRAMESIZE_QSXGA;  // Set your frame resolution here ! QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 5;              // Set your jpeg quality | 0-63 0:highest 63:lowest
    config.fb_count = 2;                  // Do not change unless you understand frame buffering
  } else {            
    // In case you dont want to use PSRAM or your ESP32S3 does not have PSRAM, use this configuration (lower resolution and quality to fit in internal RAM)
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera initialization failed with error 0x%x\n", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  if (s->id.PID == OV5640_PID) {
    // Change camera image settings here
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
}
  /*
  ESP32-S3 OV5640 WEB SERVER
  By: Simply Smart X - Albert

  DO NOT REDISTRIBUTE !
  PERSONAL OR RESEARCH USE ONLY !
  */
void handleRoot() {
  String html = "<html>\n"
                "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></head>\n"
                "<body style=\"text-align: center; font-family: sans-serif; background-color: #f0f0f0;\">\n"
                "<h1>ESP32-S3 OV5640 WEB SERVER</h1>\n"
                "<h3>By: Simply Smart X - Albert</h3>\n"
                "<h4>This Program Is Licensed Under: CC BY-NC-ND</h4>\n"
                "<div><img src=\"/capture\" style=\"max-width: 100%; height: auto; border: 2px solid #ccc; border-radius: 8px;\" /></div>\n"
                "</body>\n"
                "</html>";
  server.send(200, "text/html", html);
}

void handleCapture() {
  Serial.println("\n--- Image is being requested by client ---");
  Serial.println("Client IP: " + server.client().remoteIP().toString());
  
  // 1. Wake camera
  setCameraStandby(false);
  
  // 2. Discard the first 2 frames to let auto-exposure adjust
  for (int i = 0; i < 2; i++) {
    camera_fb_t * flush = esp_camera_fb_get();
    if (flush) esp_camera_fb_return(flush);
  }

  // 3. Capture the actual image
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    server.send(500, "text/plain", "Camera capture failed");
    setCameraStandby(true);
    return;
  }

  // 4. Copy the image into our own manually allocated PSRAM buffer
  size_t jpg_len = fb->len;
  uint8_t *jpg_buf = (uint8_t *)ps_malloc(jpg_len);
  
  if (!jpg_buf) {
    Serial.println("Failed to allocate PSRAM for image copy!");
    esp_camera_fb_return(fb);
    server.send(500, "text/plain", "Out of memory");
    setCameraStandby(true);
    return;
  }
  
  // Copy the memory, then return the frame buffer to the camera driver
  memcpy(jpg_buf, fb->buf, jpg_len);
  esp_camera_fb_return(fb);

  // 5. SLEEP THE CAMERA INSTANTLY
  // Doing this BEFORE Wi-Fi transmission isolates camera power draw from Wi-Fi power draw
  setCameraStandby(true);

  Serial.printf("Sending %u bytes\n", jpg_len);
  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Author", "Simply Smart X - Albert");
  server.sendHeader("License", "CC BY-NC-ND");

  WiFiClient client = server.client();
  const uint8_t *fb_buf = jpg_buf;
  size_t fb_len = jpg_len;
  size_t chunk_size = 4096;
  
  while (fb_len > 0) {
    if (!client.connected()) {
      Serial.println("Client disconnected mid-transfer!");
      break;
    }
    size_t to_send = (fb_len > chunk_size) ? chunk_size : fb_len;
    client.write(fb_buf, to_send);
    fb_buf += to_send;
    fb_len -= to_send;
    yield();
  }
  /*
  ESP32-S3 OV5640 WEB SERVER
  By: Simply Smart X - Albert

  DO NOT REDISTRIBUTE !
  PERSONAL OR RESEARCH USE ONLY !
  */
  // 7. Free the PSRAM buffer
  free(jpg_buf);
  Serial.println("Capture and transfer complete.");
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(1000);
  Serial.println("\n--- ESP32-S3 OV5640 WEB SERVER ---");
  Serial.println("\n--- By: Simply Smart X - Albert ---");
  Serial.println("\n--- This Code Is Licensed Under: CC BY-NC-ND ---");
  /*
  ESP32-S3 OV5640 WEB SERVER
  By: Simply Smart X - Albert

  DO NOT REDISTRIBUTE !
  PERSONAL OR RESEARCH USE ONLY !
  */
  cameraAsleep = true;

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi... ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/capture", HTTP_GET, handleCapture);
  server.begin();
  
  Serial.println("Web server started");
}
/*
ESP32-S3 OV5640 WEB SERVER
By: Simply Smart X - Albert

DO NOT REDISTRIBUTE !
PERSONAL OR RESEARCH USE ONLY !
*/
void loop() {
  server.handleClient();
  delay(10); 
}
/*
END OF FILE
ESP32-S3 OV5640 WEB SERVER
By: Simply Smart X - Albert

DO NOT REDISTRIBUTE !
PERSONAL OR RESEARCH USE ONLY !
*/
