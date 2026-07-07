// TFT_eSPI User_Setup — Profile D: ESP32-S3 + ILI9488 480x320 (SPI).
// Copy đè lên TFT_eSPI/User_Setup.h. Chân khớp profiles/esp32s3-ili9488.json.
#define ILI9488_DRIVER

#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC   14
#define TFT_RST   9

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_GFXFF
#define SMOOTH_FONT

// ILI9488 SPI 16-bit push chậm hơn; 27MHz ổn định trên đa số module S3.
#define SPI_FREQUENCY       27000000
#define SPI_READ_FREQUENCY  16000000
