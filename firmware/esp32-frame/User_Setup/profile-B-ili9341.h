// TFT_eSPI User_Setup — Profile B: ESP32 WROOM-32 + ILI9341 320x240 (SPI).
// Cách dùng: copy nội dung file này đè lên <Arduino>/libraries/TFT_eSPI/User_Setup.h
// (hoặc thêm 1 setup rồi trỏ trong User_Setup_Select.h). Chân khớp profiles/esp32-ili9341.json.
#define ILI9341_DRIVER

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4
// Đèn nền do sketch điều khiển (PIN_BL trong config.h) — đừng khai TFT_BL ở đây.

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_GFXFF
#define SMOOTH_FONT

#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
