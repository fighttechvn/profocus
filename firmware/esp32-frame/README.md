# esp32-frame — WiFi digital photo frame firmware

Biến ESP32 + màn TFT thành **khung ảnh nhận ảnh qua WiFi từ app ProFocus**. Trên cổng 8080 nó
giả lập đúng `ImageReceiverServer` của app → app đã phát hành gửi ảnh sang được **không cần
sửa app** (xem [`../../protocol/PROTOCOL.md`](../../protocol/PROTOCOL.md)).

## Cần gì
- 1 board + màn theo **Profile B / C / D** ([`../../profiles/README.md`](../../profiles/README.md)).
- Arduino IDE 2.x + **ESP32 board package** (Boards Manager → "esp32" by Espressif).
- Thư viện (Library Manager): **TFT_eSPI** (Bodmer), **TJpg_Decoder** (Bodmer), **QRCode** (ricmoo).

## Cài trong 6 bước
1. Mở thư mục này trong Arduino IDE (`esp32-frame.ino`).
2. Cài 3 thư viện trên. Cài ESP32 board package.
3. **Cấu hình màn hình:** copy `User_Setup/profile-B-ili9341.h` (hoặc C/D) đè lên
   `Documents/Arduino/libraries/TFT_eSPI/User_Setup.h`. Đây là bước hay quên nhất — sai driver
   → màn trắng/nhiễu.
4. Sửa `config.h`: điền `WIFI_SSID` / `WIFI_PASSWORD`, đặt `FRAME_PROFILE` = PROFILE_B/C/D.
5. Chọn board đúng (vd "ESP32 Dev Module" hoặc "ESP32S3 Dev Module"), chọn cổng COM, **Upload**.
6. Mở Serial Monitor 115200. Khung nối WiFi rồi hiện **IP + QR** trên màn.

## Gửi ảnh từ điện thoại
- **App ProFocus:** vào tính năng chia sẻ ảnh (Local Sync) → Recent Devices → nhập
  `IP:8080` (hoặc quét QR trên màn) → chọn ảnh → gửi. Ảnh hiện ngay, lưu vào SD.
- **Không app:** mở trình duyệt điện thoại tới `http://IP:8080`, chọn ảnh, gửi. Hoặc dùng trang
  [`../../sender`](../../sender) (tự resize trước khi gửi cho nhẹ).
- **Điều khiển:** app có nút remote → `frame.next / prev / pause` (EP14).

## Mẹo & xử lý lỗi
- **Màn trắng/nhiễu:** sai `User_Setup.h` (bước 3) hoặc sai nguồn 3V3. Kiểm tra lại driver + chân.
- **Không nối được WiFi:** ESP32 chỉ 2.4GHz. Hoặc bật `FALLBACK_SOFTAP 1` rồi nối vào AP
  `ProFocus-Frame` (pass `12345678`), gửi tới `http://192.168.4.1:8080`.
- **Ảnh không hiện nhưng app báo OK:** ảnh quá lớn/không phải JPEG baseline. Trang sender resize
  sẵn; hoặc gửi JPEG ≤ chiều rộng màn. Firmware vẫn trả `200 OK` để app không kẹt.
- **Giờ overlay sai:** bỏ comment dòng `setenv("TZ",...)` trong `setup()` cho múi giờ của bạn.
- **Không có SD:** đặt `USE_SD 0` trong config.h để dùng SPIFFS (giữ được ít ảnh hơn).

## Ảnh lưu ở đâu
Ring buffer `MAX_PHOTOS` file `/p0.jpg … /pN.jpg` trên SD (hoặc SPIFFS). Ảnh mới đè ảnh cũ nhất.
Mất điện bật lại vẫn chiếu tiếp album đã lưu.
