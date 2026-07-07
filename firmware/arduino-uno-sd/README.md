# arduino-uno-sd — Digital Photo Frame OFFLINE (Arduino Uno)

Bản nhập môn bám sát video *DIY Arduino Digital Photo Frame*: Arduino Uno + 3.5" TFT shield +
microSD. **Không WiFi** — chép ảnh BMP vào thẻ SD bằng máy tính, Uno chiếu slideshow.

> Muốn **điện thoại đẩy ảnh trực tiếp** → dùng ESP32 (`../esp32-frame`). Uno không có WiFi.

## Cần gì
- Arduino Uno R3
- 3.5" TFT LCD shield ILI9486/HX8357 (parallel 8-bit) có khe microSD
- Thẻ microSD (FAT32)
- Thư viện: **MCUFRIEND_kbv**, **Adafruit_GFX**, **SD** (kèm Arduino IDE)

## Các bước
1. Cắm shield lên Uno (không cần dây).
2. Chuẩn bị ảnh: resize **480×320**, xuất **BMP 24-bit** (Paint/Photoshop/`../../sender` →
   xem `prepare-bmp.md`), đặt tên `1.bmp`, `2.bmp`, … copy vào **gốc** thẻ SD.
3. Cắm thẻ vào khe SD trên shield.
4. Mở `arduino-uno-sd.ino`, cài 3 thư viện, chọn board **Arduino Uno**, **Upload**.
5. Khung chiếu lần lượt các ảnh, mỗi ảnh 6 giây (đổi `SLIDE_MS`).

## Lỗi thường gặp
- **Màn trắng:** sai driver — chỉnh ID trong `tft.readID()`/fallback `0x9486` cho đúng chip shield.
- **SD init failed:** thẻ chưa FAT32, hoặc `SD_CS` khác 10 (một số shield dùng chân khác).
- **Ảnh méo/màu sai:** phải là **BMP 24-bit không nén**; PNG/JPG không đọc được ở bản Uno này
  (Uno chỉ 2KB RAM, không giải JPEG nổi — đó là lý do nên lên ESP32).
- **Ảnh chỉ hiện một phần:** buffer dòng giới hạn 240px; resize ảnh ≤ 480×320 và bản này canh
  giữa, phần vượt màn bị cắt.
