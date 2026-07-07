# AGENTS.md — tích hợp khung ảnh phần cứng vào một product mới

Bạn là agent gắn **fight-frame-hardware** (khung ảnh số nhận ảnh qua WiFi) vào một app mới kiểu
ProFocus. Mục tiêu: app mới đẩy ảnh sang khung phần cứng **không cần firmware biết gì về app**.

## Nguyên tắc bất biến
1. **Cổng 8080 cố định.** Server nhận ảnh của app phải nghe 8080; firmware khung cũng 8080.
2. **Wire = hợp đồng [`protocol/PROTOCOL.md`](protocol/PROTOCOL.md)** (bám `proto/remote.proto`
   EP14, JSON legacy). `POST /upload` raw bytes + `X-Filename`; `POST /cmd` + `GET /state`.
3. **Không sửa firmware theo từng app.** Product-specific (strings, màu accent) đi qua lệnh
   `clock.setAccent`, không hardcode trong .ino.
4. **Không commit WiFi secret.** `config.h` để người dùng tự điền, thêm vào `.gitignore` nếu cần.

## App side (đẩy ảnh) — checklist
- [ ] App có `ImageReceiverServer` (hoặc client uploader) trỏ `http://IP:8080/upload`, gửi raw
      bytes + header `Content-Type` + `X-Filename` (URL-encoded). ProFocus đã có: dùng lại
      `ImageUploader` (iOS) / `ImageSenderClient` (Android).
- [ ] Màn "Recent Devices" cho nhập/scan `IP:8080` (khung hiện IP+QR trên TFT).
- [ ] (Tùy chọn) màn remote gửi `frame.next/prev/pause` qua `POST /cmd`.
- [ ] Không cần mDNS: app hiện tại tìm bằng IP. Nếu thêm auto-discover, dùng
      `_profocus-frame._tcp` (firmware đã advertise).

## Hardware side — chọn & flash
1. Chọn profile ([`profiles/README.md`](profiles/README.md)); mặc định `PROFILE_B` (ESP32+ILI9341).
2. Copy `firmware/esp32-frame/User_Setup/<profile>.h` → `TFT_eSPI/User_Setup.h`.
3. Điền WiFi + `FRAME_PROFILE` trong `config.h`. Flash. Xác minh: màn hiện IP; gửi 1 ảnh thử.

## Xác minh xong
- [ ] Gửi ảnh từ app thật → hiện trên khung, app báo thành công (nhận `200 OK`).
- [ ] `frame.next/prev/pause` hoạt động; `GET /state` trả JSON đủ field.
- [ ] Mất điện → bật lại vẫn còn album (SD/SPIFFS).
- [ ] Không có WiFi secret bị commit.
- Tham chiếu mẫu: **ProFocus** (`profocus-ios` `ImageReceiverServer.swift`, `profocus-android`
  `ImageReceiverServer.java`) — firmware giả lập đúng 2 server này.
