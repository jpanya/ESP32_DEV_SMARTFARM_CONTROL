# Web Server Dashboard - คู่มือการใช้งาน

## 📋 ภาพรวม
Web Server Dashboard สำหรับ ESP32 Smart Farm Control ที่สามารถ:
- ✅ แสดงข้อมูลเซ็นเซอร์แบบ Real-time
- ✅ ควบคุม Relay ผ่านหน้าเว็บ
- ✅ แสดงข้อมูลสภาพอากาศและคุณภาพอากาศ
- ✅ Responsive Design (รองรับมือถือ)
- ✅ ใช้ SPIFFS เพื่อประหยัด RAM

---

## 🚀 การติดตั้งและใช้งาน

### ขั้นตอนที่ 1: อัปโหลด SPIFFS
ก่อนอัปโหลดโปรแกรม ต้องอัปโหลดไฟล์ HTML ไปยัง SPIFFS ก่อน

#### วิธีที่ 1: ใช้ PlatformIO (แนะนำ)
1. เปิด VS Code Terminal
2. รันคำสั่ง:
```bash
pio run --target uploadfs
```

หรือ

```bash
platformio run --target uploadfs
```

#### วิธีที่ 2: ใช้ PlatformIO GUI
1. คลิกที่ PlatformIO icon ด้านซ้าย
2. ไปที่ **PROJECT TASKS** → **esp32doit-devkit-v1** → **Platform** → **Upload Filesystem Image**

### ขั้นตอนที่ 2: อัปโหลดโปรแกรม
```bash
pio run --target upload
```

หรือกดปุ่ม **Upload** ใน PlatformIO

---

## 🌐 การเข้าใช้งาน Web Dashboard

### 1. หา IP Address ของ ESP32
- ดูจาก OLED Display (แสดงที่หน้าจอ)
- ดูจาก Serial Monitor (115200 baud)
- ดูจาก Router settings

### 2. เปิดเว็บเบราว์เซอร์
เปิดเบราว์เซอร์แล้วพิมพ์:
```
http://[IP_ADDRESS]/
```

ตัวอย่าง:
```
http://192.168.1.100/
```

---

## 📊 ฟีเจอร์ของ Dashboard

### หน้าจอแสดงผล
1. **DS18B20 Temperature** - อุณหภูมิจากเซ็นเซอร์ DS18B20
2. **XY-MD03 Sensor** - อุณหภูมิและความชื้นจาก XY-MD03
3. **Tank Level Sensors** - สถานะน้ำในถัง (Dry/Overflow)
4. **Weather** - ข้อมูลสภาพอากาศจาก OpenWeather API
5. **Air Quality** - ค่า AQI, PM2.5, PM10
6. **Relay Controls** - ควบคุม Fan, Pump, Heater

### การควบคุม Relay
- คลิกปุ่ม **ON/OFF** เพื่อเปิด/ปิด Relay
- สถานะจะแสดงแบบ Real-time
- ปุ่มสีเขียว = ON, ปุ่มสีเทา = OFF

### Auto Update
- ข้อมูลจะอัปเดทอัตโนมัติทุกๆ **2 วินาที**
- สถานะการเชื่อมต่อแสดงที่ด้านบน (ONLINE/OFFLINE)

---

## 🔌 API Endpoints

### GET Requests

#### `/api/info`
ข้อมูลระบบ
```json
{
  "ip": "192.168.1.100",
  "ssid": "MyWiFi",
  "rssi": -45,
  "uptime": 3600
}
```

#### `/api/sensors`
ข้อมูลเซ็นเซอร์ทั้งหมด
```json
{
  "ds18b20": {
    "temperature": 25.5,
    "connected": true
  },
  "xymd03": {
    "temperature": 26.5,
    "humidity": 65.0,
    "connected": true
  },
  "iso1": {
    "active": false
  },
  "iso2": {
    "active": false
  },
  "weather": {
    "available": true,
    "city": "Bangkok",
    "temperature": 30.5,
    "humidity": 70.0,
    "feelsLike": 35.0,
    "windSpeed": 2.5,
    "description": "clear sky",
    "aqi": 2,
    "pm25": 25.5,
    "pm10": 45.0
  },
  "relays": {
    "fan": false,
    "pump": false,
    "heater": false
  }
}
```

### POST Requests

#### `/api/relay/[1-3]/on`
เปิด Relay (1=Fan, 2=Pump, 3=Heater)

**Response:**
```json
{
  "success": true,
  "relay": 1,
  "state": true
}
```

#### `/api/relay/[1-3]/off`
ปิด Relay

**Response:**
```json
{
  "success": true,
  "relay": 1,
  "state": false
}
```

---

## 🧪 ทดสอบ API ด้วย curl

### ดูข้อมูลเซ็นเซอร์
```bash
curl http://192.168.1.100/api/sensors
```

### เปิดพัดลม
```bash
curl -X POST http://192.168.1.100/api/relay/1/on
```

### ปิดพัดลม
```bash
curl -X POST http://192.168.1.100/api/relay/1/off
```

---

## 📱 การใช้งานผ่านมือถือ

Dashboard รองรับการแสดงผลบนมือถือ:
1. เชื่อมต่อมือถือกับ WiFi เดียวกับ ESP32
2. เปิดเบราว์เซอร์บนมือถือ
3. พิมพ์ IP Address ของ ESP32
4. Dashboard จะปรับขนาดอัตโนมัติให้เหมาะกับหน้าจอ

---

## 🔧 Troubleshooting

### ปัญหา: ไม่สามารถเข้า Web Dashboard ได้

**แก้ไข:**
1. ตรวจสอบว่า ESP32 เชื่อมต่อ WiFi สำเร็จ (ดูที่ OLED)
2. ตรวจสอบว่าคอมพิวเตอร์/มือถืออยู่ WiFi เดียวกับ ESP32
3. ลองใช้ IP Address แทน hostname
4. ตรวจสอบ Serial Monitor ว่า Web Server เริ่มทำงานหรือไม่

### ปัญหา: หน้าเว็บแสดง "Not Found"

**แก้ไข:**
1. ตรวจสอบว่าอัปโหลด SPIFFS แล้ว (`pio run --target uploadfs`)
2. ตรวจสอบว่าไฟล์ `data/index.html` มีอยู่
3. ลองรีสตาร์ท ESP32

### ปัญหา: ข้อมูลไม่อัปเดท

**แก้ไข:**
1. Refresh หน้าเว็บ (F5)
2. ตรวจสอบ Browser Console (F12) ดู error
3. ตรวจสอบว่า ESP32 ยังเชื่อมต่อ WiFi อยู่

### ปัญหา: กด Relay แล้วไม่ทำงาน

**แก้ไข:**
1. ตรวจสอบการต่อสาย Relay
2. ดู Serial Monitor เพื่อดู log messages
3. ตรวจสอบว่า API endpoint ส่งคำสั่งถูกต้อง

---

## 💡 Tips & Tricks

### 1. Bookmark Dashboard
- บันทึก IP Address เป็น Bookmark ในเบราว์เซอร์
- ตั้งชื่อว่า "Smart Farm Dashboard"

### 2. Add to Home Screen (Mobile)
**iOS:**
1. เปิด Safari → Dashboard URL
2. กดปุ่ม Share
3. เลือก "Add to Home Screen"

**Android:**
1. เปิด Chrome → Dashboard URL
2. Menu (⋮) → "Add to Home screen"

### 3. Multiple Device Access
- สามารถเปิด Dashboard จากหลายอุปกรณ์พร้อมกันได้
- ข้อมูลจะ sync แบบ real-time

### 4. Integration with Other Apps
ใช้ API endpoints กับแอปอื่นๆ เช่น:
- Node-RED
- Home Assistant
- IFTTT (ผ่าน Webhooks)
- Python scripts

---

## 🎨 การปรับแต่ง Dashboard

### แก้ไขสี Theme
แก้ไขไฟล์ `data/index.html` ส่วน CSS:
```css
background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
```

### เพิ่มข้อมูลใหม่
1. แก้ไข `getSensorDataJSON()` ใน `main.cpp` เพื่อเพิ่มข้อมูล
2. แก้ไข `updateData()` ใน `index.html` เพื่อแสดงผล
3. อัปโหลด SPIFFS และโปรแกรมใหม่

---

## 📊 การใช้งานหน่วยความจำ

### SPIFFS Usage
- `index.html`: ~18 KB
- Total SPIFFS: ~1.5 MB available

### RAM Usage
- ใช้ SPIFFS แทน String ใน RAM
- ลดการใช้ RAM ลงประมาณ 18 KB
- เหลือ RAM สำหรับ features อื่นๆ

---

## 🔐 Security Notes

⚠️ **คำเตือน:**
- ไม่มีการ Authentication (ทุกคนในเครือข่ายเข้าถึงได้)
- ไม่ควร expose ไปยัง Internet โดยตรง
- ใช้ในเครือข่ายท้องถิ่น (LAN) เท่านั้น

**การเพิ่มความปลอดภัย (ขั้นสูง):**
- เพิ่ม Basic Authentication
- ใช้ HTTPS (ต้องมี SSL Certificate)
- จำกัด IP Address ที่เข้าถึงได้

---

## 🎓 เอกสารเพิ่มเติม

- [ESPAsyncWebServer Documentation](https://github.com/me-no-dev/ESPAsyncWebServer)
- [SPIFFS Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html)
- [ArduinoJson Documentation](https://arduinojson.org/)

---

## ✅ Checklist การทำงาน

- [ ] อัปโหลด SPIFFS (`pio run --target uploadfs`)
- [ ] อัปโหลดโปรแกรม (`pio run --target upload`)
- [ ] ESP32 เชื่อมต่อ WiFi สำเร็จ
- [ ] เปิด Serial Monitor ดู IP Address
- [ ] เปิดเบราว์เซอร์ไปที่ IP Address
- [ ] Dashboard แสดงผลสมบูรณ์
- [ ] ทดสอบควบคุม Relay
- [ ] ข้อมูลอัปเดทอัตโนมัติ

---

**สนุกกับการใช้งาน Smart Farm Dashboard! 🌱💚**
