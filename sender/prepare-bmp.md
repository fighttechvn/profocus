# Chuẩn bị ảnh cho khung Arduino Uno (BMP 24-bit)

Profile Uno (offline) chỉ đọc **BMP 24-bit không nén**. Cách tạo:

## Cách 1 — trình duyệt (không cài gì)
Mở console trong trang bất kỳ, hoặc dùng đoạn HTML dưới (lưu `.html`, mở, kéo ảnh vào) để xuất
BMP 480×320. (ESP32 thì KHÔNG cần — nó nhận JPEG trực tiếp qua WiFi.)

```html
<input type=file id=f accept=image/*>
<a id=d>tải .bmp</a>
<script>
f.onchange=e=>{const img=new Image();img.onload=()=>{
  const W=480,H=320,cv=document.createElement('canvas');cv.width=W;cv.height=H;
  const ctx=cv.getContext('2d');ctx.fillStyle='#000';ctx.fillRect(0,0,W,H);
  const s=Math.min(W/img.width,H/img.height),w=img.width*s,h=img.height*s;
  ctx.drawImage(img,(W-w)/2,(H-h)/2,w,h);
  const d=ctx.getImageData(0,0,W,H).data,row=(W*3+3)&~3,size=54+row*H,buf=new Uint8Array(size),dv=new DataView(buf.buffer);
  dv.setUint16(0,0x424D,false);dv.setUint32(2,size,true);dv.setUint32(10,54,true);
  dv.setUint32(14,40,true);dv.setInt32(18,W,true);dv.setInt32(22,H,true);
  dv.setUint16(26,1,true);dv.setUint16(28,24,true);
  let p=54;for(let y=H-1;y>=0;y--){for(let x=0;x<W;x++){const i=(y*W+x)*4;buf[p++]=d[i+2];buf[p++]=d[i+1];buf[p++]=d[i];}p+=row-W*3;}
  const a=document.getElementById('d');a.href=URL.createObjectURL(new Blob([buf],{type:'image/bmp'}));a.download='1.bmp';a.textContent='tải 1.bmp';
};img.src=URL.createObjectURL(e.target.files[0]);};
</script>
```

## Cách 2 — Photoshop / GIMP / Paint
1. Resize canvas 480×320 (giữ tỉ lệ, nền đen phần thừa).
2. File → Export/Save As → **BMP**, chọn **24-bit**, **không** RLE/nén.
3. Đặt tên `1.bmp`, `2.bmp`, … copy vào gốc thẻ SD.

## Cách 3 — ImageMagick (dòng lệnh)
```bash
magick input.jpg -resize 480x320 -background black -gravity center -extent 480x320 \
  -type TrueColor BMP3:1.bmp
```
`BMP3:` + `-type TrueColor` đảm bảo BMP 24-bit không nén mà MCUFRIEND_kbv đọc được.
