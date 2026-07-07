// arduino-uno-sd.ino — Digital Photo Frame OFFLINE (Arduino Uno + 3.5" TFT + microSD).
//
// Bám sát video "DIY Arduino Digital Photo Frame" (MCUFRIEND_kbv). KHÔNG WiFi:
// chép ảnh BMP 24-bit vào thẻ SD bằng máy tính, Uno đọc và chiếu slideshow.
// Muốn điện thoại đẩy ảnh trực tiếp → dùng ESP32 (firmware/esp32-frame).
//
// Thư viện: MCUFRIEND_kbv, Adafruit_GFX, SD (kèm Arduino IDE).
// Chuẩn bị ảnh: resize 480x320, xuất BMP 24-bit, đặt tên 1.bmp, 2.bmp … vào gốc thẻ SD.

#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <SD.h>

MCUFRIEND_kbv tft;

#define SD_CS 10                 // chân CS của khe SD trên shield (thường 10)
#define SLIDE_MS 6000            // mỗi ảnh 6 giây
#define MAX_IMG  99              // 1.bmp .. 99.bmp

uint16_t tftW = 480, tftH = 320;

void setup() {
  Serial.begin(9600);
  uint16_t id = tft.readID();
  if (id == 0x0 || id == 0xFFFF) id = 0x9486;   // fallback ILI9486
  tft.begin(id);
  tft.setRotation(1);            // ngang
  tftW = tft.width(); tftH = tft.height();
  tft.fillScreen(0x0000);

  if (!SD.begin(SD_CS)) {
    tft.setTextColor(0xFFFF); tft.setCursor(10, 10);
    tft.print("SD init failed"); while (1);
  }
}

void loop() {
  bool any = false;
  for (int i = 1; i <= MAX_IMG; i++) {
    char name[10]; sprintf(name, "%d.bmp", i);
    if (SD.exists(name)) { any = true; drawBMP(name); delay(SLIDE_MS); }
  }
  if (!any) {
    tft.fillScreen(0x0000); tft.setTextColor(0xFFFF); tft.setCursor(10, 10);
    tft.print("No .bmp on SD"); delay(2000);
  }
}

// Đọc BMP 24-bit, canh giữa màn, vẽ từng dòng (tiết kiệm RAM Uno 2KB).
void drawBMP(const char* filename) {
  File f = SD.open(filename);
  if (!f) return;
  if (read16(f) != 0x4D42) { f.close(); return; }   // 'BM'
  read32(f); read32(f);
  uint32_t offset = read32(f);
  read32(f);
  int32_t  w = read32(f), h = read32(f);
  if (read16(f) != 1 || read16(f) != 24) { f.close(); return; }  // 1 plane, 24bpp
  read32(f);                                          // compression (0 = none)

  bool flip = true; if (h < 0) { h = -h; flip = false; }
  int rowSize = (w * 3 + 3) & ~3;                     // padding 4 byte
  int x0 = (tftW - w) / 2; if (x0 < 0) x0 = 0;
  int y0 = (tftH - h) / 2; if (y0 < 0) y0 = 0;

  uint8_t row[3 * 240];                               // buffer 1 dòng (giới hạn 240px)
  int drawW = min((int)w, 240);
  tft.fillScreen(0x0000);
  for (int y = 0; y < h && y < tftH; y++) {
    uint32_t pos = offset + (flip ? (h - 1 - y) : y) * (uint32_t)rowSize;
    f.seek(pos);
    f.read(row, min(rowSize, (int)sizeof row));
    for (int x = 0; x < drawW; x++) {
      uint8_t b = row[x * 3], g = row[x * 3 + 1], r = row[x * 3 + 2];
      tft.drawPixel(x0 + x, y0 + y, tft.color565(r, g, b));
    }
  }
  f.close();
}

uint16_t read16(File& f) { uint16_t r; r = f.read(); r |= f.read() << 8; return r; }
uint32_t read32(File& f) { uint32_t r; r = f.read(); r |= (uint32_t)f.read() << 8; r |= (uint32_t)f.read() << 16; r |= (uint32_t)f.read() << 24; return r; }
