# Digital Photo Frame — Hardware Profiles (tiered)

Năm cấu hình "digital photo frame", xếp **từ rẻ/ít tính năng → đắt/nhiều tính năng**. Chọn theo
ngân sách và nhu cầu. Mỗi profile có file JSON (`*.json`) mà firmware + trang `sdk/` đọc để hiện
BOM + sơ đồ nối dây. Sửa số chân trong JSON, đừng sửa tay firmware.

> Cảm hứng: *DIY Arduino Digital Photo Frame* (Uno + 3.5" TFT + SD, MCUFRIEND_kbv). SDK giữ bản
> đó làm bậc nhập môn và nâng lên nhận ảnh qua WiFi từ app ProFocus.

## Bậc tính năng (feature ↔ giá)

| Tier | Profile | MCU | Màn hình | Nhận ảnh từ ĐT | Lưu album | Đồng hồ overlay | Remote | Giá ~ |
|:---:|---------|-----|----------|:---:|:---:|:---:|:---:|------|
| **0 · Nhập môn (offline)** | `uno-tft35-sd` | Arduino Uno | 3.5" ILI9486 480×320 | ❌ (chép SD) | SD | ❌ | ❌ | **~$8** |
| **1 · WiFi tối giản** | `esp32-essential` | ESP32 WROOM | 2.4"/2.8" ILI9341 320×240 | ✅ WiFi | SPIFFS (~10 ảnh) | ✅ | ✅ | **~$10** |
| **2 · Khuyên dùng ⭐** | `esp32-ili9341` | ESP32 WROOM | 2.8"/3.5" ILI9341 320×240 | ✅ WiFi | microSD (60+) | ✅ | ✅ | **~$14** |
| **3 · Khung tròn** | `esp32-round-gc9a01` | ESP32 WROOM | 1.28"/2.1" GC9A01 tròn 240×240 | ✅ WiFi | SPIFFS/SD | ✅ (analog) | ✅ | **~$13** |
| **4 · Cao cấp, nét** | `esp32s3-ili9488` | ESP32-S3 (PSRAM) | 3.5"/4.0" ILI9488 480×320 | ✅ WiFi | microSD (100+) | ✅ | ✅ | **~$22** |

- **Ít tiền / học Arduino, không cần đẩy ảnh** → Tier 0.
- **Rẻ nhất mà điện thoại vẫn đẩy được ảnh** → Tier 1.
- **Cân bằng tốt nhất (đa số nên chọn)** → Tier 2 ⭐.
- **Thẩm mỹ khung tròn + đồng hồ** → Tier 3.
- **Ảnh lớn, nét, nhiều ảnh, mượt** → Tier 4.

Cùng một firmware `firmware/esp32-frame` chạy Tier 1–4 (chọn qua `config.h`): Tier 1 = `PROFILE_B`
+ `USE_SD 0`; Tier 2 = `PROFILE_B` + `USE_SD 1`; Tier 3 = `PROFILE_C`; Tier 4 = `PROFILE_D`. Tier 0
dùng firmware Arduino riêng `firmware/arduino-uno-sd` (Uno không có WiFi).

---

## Tier 0 — `uno-tft35-sd` (Offline, nhập môn)
Bám sát video gốc. Ảnh nằm trên microSD (chép bằng máy tính), Uno chiếu slideshow. Không mạng.

**BOM:** Arduino Uno R3 · 3.5" TFT shield ILI9486/HX8357 (parallel 8-bit, khe SD tích hợp) · thẻ microSD · cáp USB.
**Nối dây:** shield cắm thẳng lên Uno (không cần dây). Xem `uno-tft35-sd.json`.
**Ảnh:** resize 480×320 → BMP 24-bit → `1.bmp`, `2.bmp`… vào gốc thẻ SD (xem `../sender/prepare-bmp.md`).
**Thư viện:** `MCUFRIEND_kbv`, `Adafruit_GFX`, `SD`.

## Tier 1 — `esp32-essential` (WiFi rẻ nhất)
Giống Tier 2 nhưng bỏ thẻ SD (lưu vào SPIFFS flash nội bộ ~10 ảnh) và màn nhỏ 2.4/2.8". Rẻ nhất mà
điện thoại vẫn đẩy ảnh qua WiFi được.

**BOM:** ESP32 DevKit WROOM-32 · 2.4"/2.8" ILI9341 SPI (không cần khe SD) · cáp USB.
**Nối dây:** như bảng SPI Tier 2 (bỏ chân SD). Xem `esp32-essential.json`.
**Config:** `FRAME_PROFILE PROFILE_B`, `USE_SD 0`.

## Tier 2 — `esp32-ili9341` ⭐ (Khuyên dùng)
Khung WiFi đầy đủ: nhận ảnh, lưu album lớn trên microSD, overlay đồng hồ, remote next/prev/pause.

**BOM:** ESP32 DevKit WROOM-32 · 2.8"/3.5" ILI9341 SPI có khe microSD · thẻ microSD 8–32GB · cáp USB.

**Nối dây (SPI, xem `esp32-ili9341.json`)**

| TFT | ESP32 | TFT | ESP32 |
|-----|-------|-----|-------|
| VCC | 3V3 | SCK | GPIO 18 |
| GND | GND | SDI/MOSI | GPIO 23 |
| CS | GPIO 5 | SDO/MISO | GPIO 19 |
| RESET | GPIO 4 | LED/BL | 3V3 (hoặc GPIO 32 PWM) |
| DC/RS | GPIO 2 | SD_CS | GPIO 15 |

**Thư viện:** `TFT_eSPI`, `TJpg_Decoder`, `QRCode`, ESP32 core.

## Tier 3 — `esp32-round-gc9a01` (Khung tròn + đồng hồ)
Màn tròn GC9A01 — khung ảnh tròn kèm overlay đồng hồ analog kiểu clock của ProFocus (ảnh crop tròn).

**BOM:** ESP32 DevKit · 1.28" hoặc 2.1" GC9A01 SPI tròn 240×240 · cáp USB.
**Nối dây:** giống Tier 2 nhưng GC9A01 không có MISO/SD. Xem `esp32-round-gc9a01.json`.
**Config:** `FRAME_PROFILE PROFILE_C`.

## Tier 4 — `esp32s3-ili9488` (Cao cấp, nét)
ESP32-S3 có PSRAM giữ ảnh 480×320 full-color mượt, màn 3.5"/4.0" ILI9488, album lớn trên SD.

**BOM:** ESP32-S3 DevKitC (PSRAM 8MB) · 3.5"/4.0" ILI9488 SPI có khe microSD · thẻ microSD 8–32GB · nguồn 5V/2A.
**Nối dây:** SPI như Tier 2 nhưng MOSI=11, SCK=12, MISO=13, CS=10, DC=14, RST=9, SD_CS=21. Xem `esp32s3-ili9488.json`.
**Config:** `FRAME_PROFILE PROFILE_D`.

---

## Mua phần cứng ở đâu (đủ loại)

Giá tham khảo, đổi theo nơi bán. Ưu tiên mua module có **sẵn khe microSD trên màn** để đỡ dây.
Link là **truy vấn tìm kiếm** (bền hơn link sản phẩm hay chết) — chọn shop uy tín, đọc review.

### Board (MCU)
| Linh kiện | Dùng cho | Link tìm mua |
|-----------|----------|--------------|
| ESP32 DevKit WROOM-32 | Tier 1/2/3 | [AliExpress](https://www.aliexpress.com/wholesale?SearchText=ESP32+DevKit+WROOM-32) · [Amazon](https://www.amazon.com/s?k=ESP32+DevKit+WROOM-32) · [Shopee VN](https://shopee.vn/search?keyword=ESP32%20DevKit%20WROOM-32) · [Adafruit HUZZAH32](https://www.adafruit.com/product/3405) |
| ESP32-S3 DevKitC (PSRAM) | Tier 4 | [AliExpress](https://www.aliexpress.com/wholesale?SearchText=ESP32-S3+DevKitC+N16R8) · [Amazon](https://www.amazon.com/s?k=ESP32-S3+DevKitC) · [Espressif store](https://www.espressif.com/en/products/devkits) |
| Arduino Uno R3 (hoặc clone) | Tier 0 | [Arduino store](https://store.arduino.cc/products/arduino-uno-rev3) · [AliExpress clone](https://www.aliexpress.com/wholesale?SearchText=Arduino+Uno+R3) · [Shopee VN](https://shopee.vn/search?keyword=Arduino%20Uno%20R3) |

### Màn hình TFT
| Linh kiện | Dùng cho | Link tìm mua |
|-----------|----------|--------------|
| 2.8"/3.5" ILI9341 SPI (có khe SD) | Tier 1/2 | [AliExpress](https://www.aliexpress.com/wholesale?SearchText=2.8+ILI9341+SPI+TFT+SD) · [Amazon](https://www.amazon.com/s?k=ILI9341+2.8+SPI+TFT) · [Shopee VN](https://shopee.vn/search?keyword=ILI9341%202.8%20SPI%20TFT) |
| 3.5"/4.0" ILI9488 SPI (có khe SD) | Tier 4 | [AliExpress](https://www.aliexpress.com/wholesale?SearchText=3.5+ILI9488+SPI+TFT) · [Amazon](https://www.amazon.com/s?k=ILI9488+3.5+SPI+TFT) |
| 1.28"/2.1" GC9A01 tròn 240×240 | Tier 3 | [AliExpress](https://www.aliexpress.com/wholesale?SearchText=GC9A01+round+240+SPI) · [Amazon](https://www.amazon.com/s?k=GC9A01+round+display) · [Waveshare](https://www.waveshare.com/product/displays/lcd-oled/lcd-oled-1/1.28inch-lcd-module.htm) |
| 3.5" TFT shield ILI9486 (parallel, cho Uno) | Tier 0 | [AliExpress](https://www.aliexpress.com/wholesale?SearchText=3.5+TFT+LCD+shield+Arduino+Uno+ILI9486) · [Amazon](https://www.amazon.com/s?k=3.5+TFT+LCD+shield+Arduino+Uno) |

### Phụ kiện
| Linh kiện | Ghi chú | Link |
|-----------|---------|------|
| Thẻ microSD 8–32GB | FAT32, Tier 0/2/4 | [Amazon](https://www.amazon.com/s?k=microSD+16GB+class10) |
| Dây jumper cái-cái | nối màn ↔ ESP32 | [AliExpress](https://www.aliexpress.com/wholesale?SearchText=dupont+jumper+wire+female+female) |
| Nguồn USB 5V/1–2A + cáp | cấp điện | có sẵn ở nhà |
| Chân đế / khung gỗ / vỏ in 3D | dựng khung ảnh | [Thingiverse "ESP32 photo frame"](https://www.thingiverse.com/search?q=esp32+photo+frame) · [Printables](https://www.printables.com/search/models?q=esp32%20photo%20frame) |

### Datasheet / tài liệu tham khảo
- TFT_eSPI (driver màn): <https://github.com/Bodmer/TFT_eSPI>
- TJpg_Decoder (giải JPEG): <https://github.com/Bodmer/TJpg_Decoder>
- QRCode (ricmoo): <https://github.com/ricmoo/QRCode>
- MCUFRIEND_kbv (Uno TFT): <https://github.com/prenticedavid/MCUFRIEND_kbv>
- ESP32 Arduino core: <https://github.com/espressif/arduino-esp32>

Sau khi mua & chọn tier: mở `firmware/esp32-frame/config.h`, đặt `FRAME_PROFILE` + `USE_SD`, điền
WiFi, rồi flash. Chi tiết: `firmware/esp32-frame/README.md`.
