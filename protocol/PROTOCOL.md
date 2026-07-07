# ProFocus Frame Protocol v1 (hardware ⇄ app)

Hợp đồng mạng để **điện thoại ProFocus đẩy ảnh sang một khung ảnh phần cứng** (ESP32/Arduino)
đúng như đẩy sang một máy chạy app ProFocus. Khung phần cứng chỉ cần "trông giống" một
`ImageReceiverServer` của app → **app đã phát hành gửi được ngay, không cần sửa app**.

> Nguồn sự thật của schema điều khiển: [`../proto/remote.proto`](../proto/remote.proto)
> (EP14). Wire là **JSON legacy** — int64 in dạng number, `RemoteState` luôn in đủ field.

## 1. Transport

| | |
|---|---|
| Giao thức | HTTP/1.1 trên TCP, **cổng 8080 cố định** |
| Mạng | Cùng LAN/WiFi (LAN-only, không TLS, không auth — giống app) |
| Khám phá thiết bị | **Không mDNS.** App lưu `IP:8080` (Recent Devices) hoặc quét QR `http://IP:8080`. Khung phần cứng **phải hiện IP (và nên hiện QR)** lên màn hình để người dùng nhập/quét. |
| Địa chỉ khung | Khung lấy IPv4 non-loopback của nó, ví dụ `192.168.1.42`, phục vụ tại `http://192.168.1.42:8080` |

Khung nên bật thêm mDNS `_profocus-frame._tcp` (tên `profocus-frame`) để công cụ tương lai auto-discover — **tùy chọn, không bắt buộc** vì app hiện tại tìm bằng IP.

## 2. Endpoints

### `GET /` → trang upload HTML
Trả một trang HTML tự chứa (chọn ảnh từ trình duyệt rồi POST `/upload`). Cho phép **bất kỳ
điện thoại nào** (không cần app) mở `http://IP:8080` để gửi ảnh. Khung phần cứng nên trả một
trang tối giản tương đương.

### `POST /upload` → nhận 1 ảnh (raw bytes)
Đây là đường chính app dùng để đẩy ảnh.

**Request**
```
POST /upload HTTP/1.1
Host: 192.168.1.42:8080
Content-Type: image/jpeg           ; image/jpeg | image/png | video/mp4 | video/quicktime
X-Filename: vacation%201.jpg       ; tên file, URL-encoded
Content-Length: 248576
<… 248576 byte nhị phân của ảnh …>
```

**Response**
```
HTTP/1.1 200 OK
Content-Type: text/plain

OK
```

- Body là **raw bytes** (KHÔNG multipart). App gửi từng file một request.
- App cho tới **200 MB**/file. Phần cứng nên **stream thẳng ra SD/SPIFFS** rồi mới giải mã
  (RAM ESP32 không giữ nổi ảnh lớn). Xem `firmware/esp32-frame`.
- Phần cứng chỉ hiển thị được **JPEG/PNG**; nếu nhận video (`video/*`) thì lưu/ bỏ qua và vẫn
  trả `200 OK` để app báo thành công.
- Khuyến nghị người gửi resize ≤ chiều rộng màn (240/320/480 px) JPEG để hiện tức thì; trang
  sender kèm theo (`../sender`) tự resize trước khi gửi.

### `POST /cmd` → lệnh điều khiển (EP14, tùy chọn)
Body JSON compact (chỉ field liên quan). Trả về **state JSON** (mục `GET /state`).
```json
{"cmd":"frame.next"}     {"cmd":"frame.prev"}     {"cmd":"frame.pause"}
{"cmd":"frame.open"}     {"cmd":"clock.setAccent","hex":"#E55846"}
```
Khung ảnh xử lý các lệnh `frame.*` (next/prev/pause/open) + `clock.setAccent`; lệnh
`timer.*`/`clock.setStyle` không áp dụng cho khung thì **bỏ qua nhưng vẫn trả state**.

### `GET /state` → trạng thái hiện tại (EP14, tùy chọn)
Luôn in **đủ** field (wire legacy). Với khung ảnh:
```json
{"screen":"frame","remaining":0,"total":0,"running":true,"armed":false,"style":"flip"}
```
- `screen`: `"frame"` khi đang chiếu ảnh (hoặc `"receive"` khi ở màn chờ nhận).
- `running`: slideshow đang chạy hay đã pause. Các field timer để 0/false.

## 3. Lệnh áp dụng cho khung ảnh (subset EP14)

| cmd | field | Khung ảnh làm gì |
|-----|-------|------------------|
| `frame.open` | — | Vào chế độ chiếu ảnh |
| `frame.next` | — | Ảnh kế |
| `frame.prev` | — | Ảnh trước |
| `frame.pause` | — | Bật/tắt tự chuyển ảnh |
| `clock.setAccent` | `hex` `#RRGGBB` | Đổi màu overlay đồng hồ/viền |
| `receive.open` | — | Hiện màn "chờ nhận ảnh" + IP/QR |
| `timer.*`, `clock.setStyle` | — | Bỏ qua (vẫn trả `/state`) |

## 4. Luồng dùng thực tế

```
[ Điện thoại · app ProFocus ]                 [ Khung ảnh phần cứng · ESP32 ]
  1. Khung bật nguồn, nối WiFi, hiện "192.168.1.42:8080" + QR trên TFT
  2. App → Share/Local Sync → Recent Devices → nhập/scan 192.168.1.42:8080
  3. App: POST /upload (image/jpeg, X-Filename) ──────────────────►  lưu ra SD, giải JPEG, hiện lên màn
  4. App (tùy chọn) POST /cmd {"cmd":"frame.next"} ─────────────►  chuyển ảnh; trả /state
```

Không có app? Mở trình duyệt điện thoại tới `http://192.168.1.42:8080`, chọn ảnh, gửi. Hoặc
mở `../sender/index.html` (trang gửi có resize sẵn).

## 5. Tương thích / lưu ý triển khai phần cứng
- **Cổng 8080** — không đổi, nếu đổi app không gửi được.
- Parse request theo byte: đọc dòng request + headers tới `\r\n\r\n`, rồi đọc đúng
  `Content-Length` byte của body (đừng nuốt body khi parse header) — giống server iOS/Android.
- Header không phân biệt hoa/thường; `X-Filename` URL-decode để lấy tên.
- Trả `200 OK` **sau khi** đã nhận hết body, kể cả khi không hiển thị được (video) — để app
  không báo lỗi và không gửi lại.
- Nhiều file: app mở nhiều request tuần tự; giữ server chấp nhận liên tiếp (SO_REUSEADDR).
