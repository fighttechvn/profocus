// TFT_eSPI User_Setup — Profile C: ESP32 WROOM-32 + GC9A01 round 240x240 (SPI).
// Copy đè lên TFT_eSPI/User_Setup.h. Chân khớp profiles/esp32-round-gc9a01.json.
#define GC9A01_DRIVER

#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4
// GC9A01 không có MISO/SD. Đèn nền nối 3V3 (hoặc PIN_BL trong config.h).

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_GFXFF
#define SMOOTH_FONT

#define SPI_FREQUENCY 40000000
