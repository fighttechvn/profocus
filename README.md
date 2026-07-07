# ProFocus Frame SDK — DIY WiFi Digital Photo Frame

Turn a **~$10 ESP32 + a small TFT screen** into a real **digital photo frame** that your phone
sends photos to over WiFi — straight from the [ProFocus](https://profocus.fighttech.vn) app,
**with zero app changes**. Also includes an offline Arduino Uno build for the classic
*"DIY Arduino Digital Photo Frame"* project.

> The firmware makes a cheap microcontroller look **exactly like ProFocus's built-in photo
> receiver** (a tiny HTTP server on port `8080`). To the app, your DIY frame is just another
> ProFocus screen on the network. Send a photo → it appears on the frame instantly.

- 📷 **Receive photos over WiFi** from the ProFocus app (or any phone browser)
- 🖼️ **Slideshow** with an optional clock overlay; album saved to SD, survives reboots
- 🎛️ **Remote** next / prev / pause from the app
- 🔒 **LAN-only, no cloud, no account** — same privacy model as the app
- 🧰 **5 hardware tiers** from $8 offline to a $22 hi-res frame
- 📄 The whole integration is **~4 HTTP calls** — see [`protocol/PROTOCOL.md`](protocol/PROTOCOL.md)

---

## 1. Pick your hardware (cheap → premium)

| Tier | Board + screen | Phone can push? | Album | ~Cost |
|:---:|----------------|:---:|-------|------|
| **0 · Starter (offline)** | Arduino Uno + 3.5" TFT + SD | ❌ SD card only | SD | **$8** |
| **1 · Essential WiFi** | ESP32 + 2.4"/2.8" ILI9341 | ✅ WiFi | SPIFFS (~10) | **$10** |
| **2 · Recommended ⭐** | ESP32 + 2.8"/3.5" ILI9341 + microSD | ✅ WiFi | SD (60+) | **$14** |
| **3 · Round clock** | ESP32 + 1.28"/2.1" GC9A01 round | ✅ WiFi | SPIFFS/SD | **$13** |
| **4 · Premium (hi-res)** | ESP32-S3 + 3.5"/4.0" ILI9488 + microSD | ✅ WiFi | SD (100+) | **$22** |

Full bill of materials, wiring tables **and where to buy each part** →
[`profiles/README.md`](profiles/README.md). Machine-readable specs in
[`profiles/*.json`](profiles/).

**New here? Buy the Tier 2 parts.** The Arduino Uno (Tier 0) has no WiFi, so the phone can't push
to it — it's the offline learning build.

### Quick shopping list (Tier 2, recommended)
- ESP32 DevKit WROOM-32 — [AliExpress](https://www.aliexpress.com/wholesale?SearchText=ESP32+DevKit+WROOM-32) · [Amazon](https://www.amazon.com/s?k=ESP32+DevKit+WROOM-32) · [Adafruit](https://www.adafruit.com/product/3405)
- 2.8"/3.5" ILI9341 SPI TFT with microSD slot — [AliExpress](https://www.aliexpress.com/wholesale?SearchText=2.8+ILI9341+SPI+TFT+SD) · [Amazon](https://www.amazon.com/s?k=ILI9341+2.8+SPI+TFT)
- microSD card 8–32GB, jumper wires, USB 5V cable

---

## 2. Flash the firmware (Tier 1–4, ESP32)

1. Install **Arduino IDE 2.x** + the **ESP32 board package** (Boards Manager → "esp32" by Espressif).
2. Install libraries (Library Manager): **TFT_eSPI** (Bodmer), **TJpg_Decoder** (Bodmer), **QRCode** (ricmoo).
3. Open [`firmware/esp32-frame/esp32-frame.ino`](firmware/esp32-frame).
4. **Configure the display:** copy `firmware/esp32-frame/User_Setup/profile-B-ili9341.h` (or C/D) over
   `Arduino/libraries/TFT_eSPI/User_Setup.h`. *(Most-forgotten step — wrong driver = white screen.)*
5. Edit [`firmware/esp32-frame/config.h`](firmware/esp32-frame/config.h): set `WIFI_SSID` /
   `WIFI_PASSWORD`, pick `FRAME_PROFILE` (and `USE_SD 0` for Tier 1).
6. Select your board (e.g. "ESP32 Dev Module"), pick the COM port, **Upload**.

The frame boots, joins WiFi, and shows its **IP address + a QR code** on screen.

Offline Arduino Uno build (Tier 0): see [`firmware/arduino-uno-sd/`](firmware/arduino-uno-sd).

---

## 3. Send photos from your phone

- **From the ProFocus app:** open photo sharing (Local Sync) → Recent Devices → enter the frame's
  `IP:8080` (or scan the QR shown on the frame) → pick photos → send. They appear immediately.
- **From any phone, no app:** open `http://IP:8080` in the browser and upload there — or use the
  bundled [`sender/`](sender/index.html) page (resizes before sending, plus next/prev/pause buttons).
- **Remote control:** the app's remote sends `frame.next` / `frame.prev` / `frame.pause`.

---

## 4. Integrate into your own app

Your app only needs to speak the [ProFocus Frame Protocol](protocol/PROTOCOL.md) — the same
contract the ProFocus app already uses:

```http
POST /upload HTTP/1.1            # phone → frame: one photo
Host: 192.168.1.42:8080
Content-Type: image/jpeg
X-Filename: vacation.jpg
<raw JPEG bytes>                 # → 200 OK "OK", frame displays it

POST /cmd  {"cmd":"frame.next"}  # remote: next / prev / pause
GET  /state                      # {"screen":"frame","running":true,...}
GET  /                           # a browser upload page (any phone)
```

- **Port `8080` is fixed.** Discovery is by IP or QR (the frame prints both on the TFT) — no cloud,
  no mDNS required (the firmware advertises `_profocus-frame._tcp` too, optionally).
- Send **raw bytes** (not multipart), one photo per request, with an `X-Filename` header.
- Command/state wire format follows [`proto/remote.proto`](proto/remote.proto) (EP14, legacy JSON).

Integrating the shared FightTech libraries into a whole app? See [`AGENTS.md`](AGENTS.md).

---

## Repository layout

| Path | What |
|------|------|
| [`protocol/PROTOCOL.md`](protocol/PROTOCOL.md) | The network contract (port 8080, `/upload`, `/cmd`, `/state`) |
| [`profiles/`](profiles/README.md) | 5 hardware tiers — BOM, wiring, buy links, JSON specs |
| [`firmware/esp32-frame/`](firmware/esp32-frame/README.md) | ESP32 WiFi frame firmware (main) |
| [`firmware/arduino-uno-sd/`](firmware/arduino-uno-sd/README.md) | Arduino Uno offline SD slideshow |
| [`sender/`](sender/index.html) | Browser reference sender (resize + push) |
| [`AGENTS.md`](AGENTS.md) | Guide for AI agents integrating the SDK into a new app |

## Design rules
- **No secrets in code.** Only your WiFi SSID/password (in `config.h`, don't commit real values).
- **Don't change port 8080** — the app expects it.
- **LAN-only, no auth** — keep it on your home WiFi; never expose port 8080 to the internet.

## Links
- App & guide: <https://profocus.fighttech.vn/sdk/>
- Launch post: <https://profocus.fighttech.vn/blog/diy-digital-photo-frame/>
- Libraries: [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) · [TJpg_Decoder](https://github.com/Bodmer/TJpg_Decoder) · [QRCode](https://github.com/ricmoo/QRCode) · [MCUFRIEND_kbv](https://github.com/prenticedavid/MCUFRIEND_kbv)

## License
[MIT](LICENSE) © Fighttech. Build freely, remix, and share your frame.
