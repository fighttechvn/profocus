// esp32-frame.ino — ProFocus digital photo frame (ESP32).
//
// Trở thành một "ImageReceiverServer" trên cổng 8080: app ProFocus (iOS/Android) đẩy ảnh
// tới đây y như đẩy sang một máy chạy app — KHÔNG cần sửa app. Xem ../../protocol/PROTOCOL.md.
//
// Luồng: nối WiFi → hiện IP+QR trên TFT → nhận POST /upload (raw JPEG) → lưu SD → giải mã →
// slideshow. Hỗ trợ POST /cmd (frame.next/prev/pause) + GET /state (EP14).
//
// Thư viện (Arduino IDE → Library Manager):
//   TFT_eSPI (Bodmer)  ·  TJpg_Decoder (Bodmer)  ·  QRCode (ricmoo)
//   ESP32 core (WiFi/WebServer/SD/SPIFFS/ESPmDNS/time đã kèm).
// Cấu hình chân màn hình: copy User_Setup/<profile>.h vào TFT_eSPI/User_Setup.h (xem README).

#include "config.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include <qrcode.h>
#include <time.h>
#if USE_SD
  #include <SD.h>
  #include <SPI.h>
  #define FSYS SD
#else
  #include <SPIFFS.h>
  #define FSYS SPIFFS
#endif

TFT_eSPI tft = TFT_eSPI();
WiFiServer server(FRAME_PORT);

// Trạng thái khung (map sang RemoteState EP14).
static int   photoCount = 0;      // số ảnh đang có trên FS
static int   photoIndex = 0;      // ảnh đang hiển thị
static int   writeSlot  = 0;      // ô kế để ghi (ring buffer)
static bool  slideshow  = true;   // đang tự chuyển ảnh?
static bool  receiveMode = true;  // đang ở màn "chờ nhận" (chưa có ảnh)
static uint32_t accent  = 0xE55846; // màu overlay (#RRGGBB) — coral ProFocus
static uint32_t lastSlide = 0;

static String pathFor(int slot) { return "/p" + String(slot) + ".jpg"; }

// ── TJpg_Decoder: đẩy từng block đã giải mã lên TFT ──────────────────────────
bool tftOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bmp) {
  if (y >= tft.height()) return false;
  tft.pushImage(x, y, w, h, bmp);
  return true;
}

// ── Boot screen: IP + QR (app quét QR = http://IP:8080) ─────────────────────
void drawWelcome(const String& ip) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("ProFocus Frame", tft.width() / 2, 6, 2);

  String url = "http://" + ip + ":" + String(FRAME_PORT);
  QRCode qr; uint8_t qrData[qrcode_getBufferSize(3)];
  qrcode_initText(&qr, qrData, 3, ECC_LOW, url.c_str());
  int scale = min((tft.width() - 20), (tft.height() - 60)) / qr.size;
  if (scale < 1) scale = 1;
  int ox = (tft.width() - qr.size * scale) / 2;
  int oy = 34;
  tft.fillRect(ox - 4, oy - 4, qr.size * scale + 8, qr.size * scale + 8, TFT_WHITE);
  for (uint8_t y = 0; y < qr.size; y++)
    for (uint8_t x = 0; x < qr.size; x++)
      if (qrcode_getModule(&qr, x, y))
        tft.fillRect(ox + x * scale, oy + y * scale, scale, scale, TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(ip + ":" + String(FRAME_PORT), tft.width() / 2, oy + qr.size * scale + 8, 2);
  tft.drawString("Send photos from ProFocus", tft.width() / 2, tft.height() - 16, 1);
}

// ── Hiển thị 1 ảnh từ FS, scale vừa màn, canh giữa ──────────────────────────
void showPhoto(int slot) {
  String path = pathFor(slot);
  if (!FSYS.exists(path)) return;
  uint16_t w = 0, h = 0;
#if USE_SD
  TJpgDec.getSdJpgSize(&w, &h, path.c_str());
#else
  TJpgDec.getFsJpgSize(&w, &h, path.c_str(), FSYS);
#endif
  uint8_t scale = 1;
  while ((w / scale > tft.width() || h / scale > tft.height()) && scale < 8) scale <<= 1;
  TJpgDec.setJpgScale(scale);
  int ox = (tft.width()  - w / scale) / 2;
  int oy = (tft.height() - h / scale) / 2;
  tft.fillScreen(TFT_BLACK);
#if USE_SD
  TJpgDec.drawSdJpg(ox < 0 ? 0 : ox, oy < 0 ? 0 : oy, path.c_str());
#else
  TJpgDec.drawFsJpg(ox < 0 ? 0 : ox, oy < 0 ? 0 : oy, path.c_str(), FSYS);
#endif
  if (SHOW_CLOCK) drawClockOverlay();
  receiveMode = false;
}

void drawClockOverlay() {
  struct tm t;
  if (!getLocalTime(&t, 5)) return;
  char buf[6]; strftime(buf, sizeof buf, "%H:%M", &t);
  uint16_t c = tft.color565((accent >> 16) & 0xFF, (accent >> 8) & 0xFF, accent & 0xFF);
  tft.setTextColor(c, TFT_BLACK);
  tft.setTextDatum(BR_DATUM);
  tft.drawString(buf, tft.width() - 6, tft.height() - 4, 4);
}

// ── Quét FS lúc boot để biết có sẵn bao nhiêu ảnh ───────────────────────────
void scanPhotos() {
  photoCount = 0;
  for (int i = 0; i < MAX_PHOTOS; i++) if (FSYS.exists(pathFor(i))) photoCount++;
  writeSlot = photoCount % MAX_PHOTOS;
}

// ── HTTP helpers ────────────────────────────────────────────────────────────
String readLine(WiFiClient& c) {
  String s; uint32_t t = millis();
  while (c.connected() && millis() - t < 3000) {
    if (c.available()) {
      char ch = c.read();
      if (ch == '\n') break;
      if (ch != '\r') s += ch;
      t = millis();
    }
  }
  return s;
}

String urlDecode(const String& s) {
  String o; char a, b;
  for (uint32_t i = 0; i < s.length(); i++) {
    if (s[i] == '%' && i + 2 < s.length()) {
      a = s[i + 1]; b = s[i + 2];
      a = a <= '9' ? a - '0' : (a | 0x20) - 'a' + 10;
      b = b <= '9' ? b - '0' : (b | 0x20) - 'a' + 10;
      o += char(a * 16 + b); i += 2;
    } else if (s[i] == '+') o += ' ';
    else o += s[i];
  }
  return o;
}

String stateJson() {
  String screen = receiveMode ? "receive" : "frame";
  String style = "flip";
  return "{\"screen\":\"" + screen + "\",\"remaining\":0,\"total\":0,\"running\":" +
         (slideshow ? "true" : "false") + ",\"armed\":false,\"style\":\"" + style + "\"}";
}

void sendText(WiFiClient& c, const char* status, const char* type, const String& body) {
  c.print("HTTP/1.1 "); c.print(status); c.print("\r\n");
  c.print("Content-Type: "); c.print(type); c.print("\r\n");
  c.print("Content-Length: "); c.print(body.length()); c.print("\r\n");
  c.print("Connection: close\r\n\r\n");
  c.print(body);
}

const char UPLOAD_PAGE[] PROGMEM =
  "<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>"
  "<title>ProFocus Frame</title><body style='font-family:sans-serif;background:#0E0E10;color:#fff;text-align:center;padding:24px'>"
  "<h2>ProFocus Frame</h2><p>Choose photos to send to this frame.</p>"
  "<input id=f type=file accept='image/*' multiple><p id=s></p>"
  "<script>f.onchange=async e=>{for(const file of e.target.files){s.textContent='Sending '+file.name+'…';"
  "await fetch('/upload',{method:'POST',headers:{'Content-Type':file.type||'image/jpeg','X-Filename':encodeURIComponent(file.name)},body:file});}"
  "s.textContent='Done \\u2713';};</script></body>";

// Nhận raw body của POST /upload, stream ra file trên FS.
void handleUpload(WiFiClient& c, long len, const String& filename) {
  String path = pathFor(writeSlot);
  File f = FSYS.open(path, FILE_WRITE);
  if (!f) { sendText(c, "500 Internal Server Error", "text/plain", "ERR"); return; }
  uint8_t buf[1024];
  long remaining = len; uint32_t t = millis();
  while (remaining > 0 && c.connected() && millis() - t < 20000) {
    int avail = c.available();
    if (avail <= 0) { delay(1); continue; }
    int n = c.read(buf, min((long)sizeof buf, min((long)avail, remaining)));
    if (n <= 0) continue;
    f.write(buf, n); remaining -= n; t = millis();
  }
  f.close();
  sendText(c, "200 OK", "text/plain", "OK");   // trả OK dù sau đó có hiển thị được hay không

  if (photoCount < MAX_PHOTOS) photoCount++;
  photoIndex = writeSlot;
  writeSlot = (writeSlot + 1) % MAX_PHOTOS;
  showPhoto(photoIndex);
  lastSlide = millis();
}

// Đọc & xử lý 1 request.
void serveClient(WiFiClient& c) {
  String reqLine = readLine(c);
  if (reqLine.length() == 0) return;
  int sp1 = reqLine.indexOf(' '), sp2 = reqLine.indexOf(' ', sp1 + 1);
  String method = reqLine.substring(0, sp1);
  String path   = reqLine.substring(sp1 + 1, sp2);

  long contentLen = 0; String filename = "photo.jpg";
  for (;;) {
    String h = readLine(c);
    if (h.length() == 0) break;                 // hết header
    int colon = h.indexOf(':'); if (colon < 0) continue;
    String k = h.substring(0, colon); k.toLowerCase();
    String v = h.substring(colon + 1); v.trim();
    if (k == "content-length") contentLen = v.toInt();
    else if (k == "x-filename") filename = urlDecode(v);
  }

  if (method == "POST" && path == "/upload") {
    handleUpload(c, contentLen, filename);
  } else if (path == "/state") {
    sendText(c, "200 OK", "application/json", stateJson());
  } else if (method == "POST" && path == "/cmd") {
    String body; while (contentLen-- > 0 && c.connected()) { while (!c.available() && c.connected()) delay(1); body += (char)c.read(); }
    handleCmd(body);
    sendText(c, "200 OK", "application/json", stateJson());
  } else if (path == "/") {
    sendText(c, "200 OK", "text/html", String(UPLOAD_PAGE));
  } else {
    sendText(c, "404 Not Found", "text/plain", "not found");
  }
}

void handleCmd(const String& json) {
  int i = json.indexOf("\"cmd\""); if (i < 0) return;
  int q = json.indexOf('"', json.indexOf(':', i) + 1);
  String cmd = json.substring(q + 1, json.indexOf('"', q + 1));
  if (cmd == "frame.next")      { nextPhoto(+1); }
  else if (cmd == "frame.prev") { nextPhoto(-1); }
  else if (cmd == "frame.pause"){ slideshow = !slideshow; }
  else if (cmd == "frame.open") { slideshow = true; if (photoCount) showPhoto(photoIndex); }
  else if (cmd == "receive.open"){ receiveMode = true; drawWelcome(WiFi.localIP().toString()); }
  else if (cmd == "clock.setAccent") {
    int h = json.indexOf('#'); if (h > 0) accent = strtol(json.substring(h + 1, h + 7).c_str(), nullptr, 16);
  }
}

void nextPhoto(int dir) {
  if (photoCount == 0) return;
  photoIndex = (photoIndex + dir + photoCount) % photoCount;
  showPhoto(photoIndex);
  lastSlide = millis();
}

// ── setup / loop ────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  if (PIN_BL) { pinMode(PIN_BL, OUTPUT); digitalWrite(PIN_BL, HIGH); }
  tft.init();
  tft.setRotation(SCREEN_W > SCREEN_H ? 1 : 0);
  tft.fillScreen(TFT_BLACK);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tftOutput);

#if USE_SD
  if (!SD.begin(PIN_SD_CS)) { Serial.println("SD fail"); }
#else
  SPIFFS.begin(true);
#endif
  scanPhotos();

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Connecting WiFi...", tft.width() / 2, tft.height() / 2, 2);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) delay(250);

  String ip;
#if FALLBACK_SOFTAP
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ProFocus-Frame", "12345678");
    ip = WiFi.softAPIP().toString();       // 192.168.4.1
  } else
#endif
  {
    ip = WiFi.localIP().toString();
    configTime(0, 0, "pool.ntp.org");       // UTC; đặt TZ ở dòng dưới nếu muốn giờ địa phương
    // setenv("TZ","ICT-7",1); tzset();      // ví dụ giờ VN (UTC+7)
    if (MDNS.begin("profocus-frame")) MDNS.addService("profocus-frame", "tcp", FRAME_PORT);
  }

  server.begin();
  if (photoCount > 0) { photoIndex = 0; showPhoto(0); }
  else drawWelcome(ip);
  lastSlide = millis();
}

void loop() {
  WiFiClient c = server.available();
  if (c) { serveClient(c); c.stop(); }

  if (slideshow && photoCount > 1 && millis() - lastSlide > SLIDE_INTERVAL_MS) {
    nextPhoto(+1);
  }
}
