// config.h — chọn profile phần cứng + WiFi. Sửa file NÀY, đừng sửa .ino.
//
// Profile ↔ sơ đồ chân xem ../../profiles/*.json và ../../profiles/README.md.
#pragma once

// ── 1) WiFi ────────────────────────────────────────────────────────────────
// Điền SSID/mật khẩu WiFi nhà (2.4GHz — ESP32 không bắt 5GHz).
#define WIFI_SSID     "YOUR_WIFI"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Nếu không có WiFi nhà: đặt 1 để khung tự phát AP tên "ProFocus-Frame" (pass 12345678),
// điện thoại nối vào AP đó rồi gửi tới http://192.168.4.1:8080.
#define FALLBACK_SOFTAP 1

// ── 2) Chọn profile ─────────────────────────────────────────────────────────
#define PROFILE_B  1   // ESP32 + ILI9341 320x240  (khuyến nghị)
#define PROFILE_C  2   // ESP32 + GC9A01 tròn 240x240
#define PROFILE_D  3   // ESP32-S3 + ILI9488 480x320

#define FRAME_PROFILE PROFILE_B

// ── 3) Cổng giao thức (ĐỪNG đổi — app ProFocus cố định 8080) ─────────────────
#define FRAME_PORT 8080

// ── 4) Hành vi slideshow ────────────────────────────────────────────────────
#define SLIDE_INTERVAL_MS 8000   // thời gian mỗi ảnh (ms)
#define MAX_PHOTOS        60     // số ảnh giữ trong ring buffer trên SD/SPIFFS
#define SHOW_CLOCK        1      // overlay đồng hồ số góc dưới (cần NTP, có WiFi)

// ── 5) Lưu trữ ──────────────────────────────────────────────────────────────
// 1 = dùng thẻ microSD (khuyên dùng, giữ nhiều ảnh); 0 = SPIFFS flash nội bộ (ít ảnh).
#define USE_SD 1

// ── Sơ đồ chân theo profile (khớp profiles/*.json) ──────────────────────────
// TFT_eSPI đọc chân từ User_Setup của nó, KHÔNG từ đây — xem User_Setup/README.md.
// Các #define dưới chỉ dùng cho chân SD_CS + đèn nền do sketch tự quản.
#if FRAME_PROFILE == PROFILE_B
  #define SCREEN_W 320
  #define SCREEN_H 240
  #define PIN_SD_CS 15
  #define PIN_BL    32     // đèn nền PWM (0 = nối thẳng 3V3, bỏ qua)
  #define SCREEN_ROUND 0
#elif FRAME_PROFILE == PROFILE_C
  #define SCREEN_W 240
  #define SCREEN_H 240
  #define PIN_SD_CS 0      // GC9A01 thường không có SD
  #define PIN_BL    0
  #define SCREEN_ROUND 1
#elif FRAME_PROFILE == PROFILE_D
  #define SCREEN_W 480
  #define SCREEN_H 320
  #define PIN_SD_CS 21
  #define PIN_BL    0
  #define SCREEN_ROUND 0
#endif
