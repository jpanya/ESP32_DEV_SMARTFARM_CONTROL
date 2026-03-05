# 📱 Telegram Notification Guide - Smart Farm Control

## 🌟 ภาพรวม

ระบบแจ้งเตือนผ่าน Telegram ช่วยให้คุณรับข้อมูลสำคัญของ Smart Farm Control แบบเรียลไทม์ผ่านแอป Telegram บนมือถือของคุณ

## 🎯 ฟีเจอร์การแจ้งเตือน

ระบบสามารถแจ้งเตือนเหตุการณ์ต่างๆ ได้ดังนี้:

### 1. 🔥 แจ้งเตือนอุณหภูมิสูง
- แจ้งเตือนเมื่ออุณหภูมิเกินค่าที่กำหนด (TEMP_FAN_ON)
- แสดงอุณหภูมิปัจจุบันและสถานะพัดลม
- มี Cooldown 5 นาที เพื่อป้องกันการส่งข้อความซ้ำ

### 2. ❄️ แจ้งเตือนอุณหภูมิต่ำ
- แจ้งเตือนเมื่ออุณหภูมิต่ำกว่าค่าที่กำหนด (TEMP_HEATER_ON)
- แสดงอุณหภูมิปัจจุบันและสถานะฮีตเตอร์

### 3. 💧 แจ้งเตือนความชื้นสูง
- แจ้งเตือนเมื่อความชื้นเกินค่าที่กำหนด (HUM_MAX)
- แสดงความชื้นปัจจุบันและสถานะพัดลม

### 4. 🏜️ แจ้งเตือนความชื้นต่ำ
- แจ้งเตือนเมื่อความชื้นต่ำกว่าค่าที่กำหนด (HUM_MIN)
- แสดงความชื้นปัจจุบันและสถานะปั๊มน้ำ

### 5. 🚰 แจ้งเตือนระดับน้ำ
- แจ้งเตือนเมื่อแท้งค์น้ำแห้ง (ISO1 active)
- แจ้งเตือนเมื่อน้ำล้น (ISO2 active)

### 6. ⚡ แจ้งเตือนสถานะรีเลย์
- แจ้งเตือนเมื่อสถานะของรีเลย์เปลี่ยนแปลง (เปิด/ปิด)
- รองรับทั้ง 3 รีเลย์: พัดลม, ปั๊มน้ำ, ฮีตเตอร์

### 7. 🚀 แจ้งเตือนเมื่อระบบเริ่มต้น
- แจ้งเตือนเมื่อ ESP32 รีสตาร์ทหรือเริ่มทำงาน
- แสดงข้อมูล IP Address และ WiFi Signal

### 8. 📊 ข้อมูลระบบแบบเต็ม
- ส่งได้ผ่านปุ่มทดสอบใน Dashboard
- แสดงข้อมูลครบถ้วน: อุณหภูมิ, ความชื้น, สถานะรีเลย์, ระดับน้ำ, ระบบอัตโนมัติ

---

## 🔧 การติดตั้งและตั้งค่า

### ขั้นตอนที่ 1: สร้าง Telegram Bot

1. เปิด Telegram และค้นหา **@BotFather**
2. พิมพ์คำสั่ง `/newbot`
3. ตั้งชื่อบอท เช่น "My Smart Farm Bot"
4. ตั้งยูสเซอร์เนม เช่น "MySmartFarm_bot" (ต้องลงท้ายด้วย bot)
5. คัดลอก **Bot Token** ที่ได้รับ (ตัวอย่าง: `123456789:ABCdefGHIjklMNOpqrsTUVwxyz`)

### ขั้นตอนที่ 2: หา Chat ID

1. เปิด Telegram และค้นหา **@userinfobot**
2. กดปุ่ม `/start`
3. บอทจะแสดง **Chat ID** ของคุณ (ตัวอย่าง: `987654321`)
4. คัดลอก Chat ID นี้

**หมายเหตุ:** สำหรับ Group Chat
- เพิ่มบอทเข้ากรุ๊ป
- ส่งข้อความใดก็ได้ใน Group
- เข้า https://api.telegram.org/bot`YOUR_BOT_TOKEN`/getUpdates
- ดู Chat ID ของกรุ๊ป (จะเป็นเลขติดลบ เช่น `-1001234567890`)

### ขั้นตอนที่ 3: ตั้งค่าใน Dashboard

1. เปิด Web Dashboard ของ Smart Farm (http://ESP32-IP-Address/)
2. ไปที่หน้า **Settings**
3. เลื่อนลงมาหาส่วน **"Telegram Bot Settings"**
4. กรอกข้อมูล:
   - ✅ **เปิดใช้งาน Telegram**: เปิด
   - 🤖 **Bot Token**: วาง Bot Token จาก @BotFather
   - 💬 **Chat ID**: วาง Chat ID ของคุณ
5. เลือกประเภทการแจ้งเตือนที่ต้องการ:
   - ✅ แจ้งเตือนอุณหภูมิสูง
   - ✅ แจ้งเตือนอุณหภูมิต่ำ
   - ✅ แจ้งเตือนความชื้นสูง
   - ✅ แจ้งเตือนความชื้นต่ำ
   - ✅ แจ้งเตือนระดับน้ำ
   - ✅ แจ้งเตือนสถานะรีเลย์
   - ✅ แจ้งเตือนเมื่อระบบเริ่มต้น
6. กดปุ่ม **"📨 ส่งข้อความทดสอบ"** เพื่อทดสอบ
7. ตรวจสอบ Telegram ของคุณ - ควรได้รับข้อความจากบอท
8. กดปุ่ม **"💾 บันทึกการตั้งค่า"**

---

## 📝 ตัวอย่างข้อความแจ้งเตือน

### ข้อความเมื่อระบบเริ่มต้น
```
🌱 Smart Farm Alert

🚀 ระบบ Smart Farm เริ่มทำงาน

✅ ระบบพร้อมใช้งาน
📍 IP: 192.168.1.100
🌐 WiFi Signal: -45 dBm
```

### ข้อความแจ้งเตือนอุณหภูมิสูง
```
🌱 Smart Farm Alert

🔥 แจ้งเตือน: อุณหภูมิสูง!

อุณหภูมิปัจจุบัน: 32.5°C
เกินค่าที่กำหนด: 30.0°C

💨 พัดลมกำลังเปิด
```

### ข้อความแจ้งเตือนระดับน้ำ
```
🌱 Smart Farm Alert

🚰 แจ้งเตือน: ระดับน้ำ!

⚠️ แท้งค์น้ำแห้ง!
กรุณาเติมน้ำ
```

### ข้อความสถานะรีเลย์เปลี่ยน
```
🌱 Smart Farm Alert

🟢 พัดลม ถูกเปิด

เวลา: 12345 วินาที
```

### ข้อความข้อมูลระบบแบบเต็ม (กดปุ่มทดสอบ)
```
🌱 Smart Farm Alert

📊 ข้อมูลระบบ Smart Farm

🌐 WiFi
• IP: 192.168.1.100
• Signal: -45 dBm

🌡️ เซ็นเซอร์อุณหภูมิ
• DS18B20: 25.3°C
• XY-MD03: 26.5°C

💧 ความชื้น
• XY-MD03: 68.2%

⚡ สถานะรีเลย์
• พัดลม: 🔴 ปิด
• ปั๊มน้ำ: 🔴 ปิด
• ฮีตเตอร์: 🔴 ปิด

🚰 ระดับน้ำ
• ✅ ปกติ

🤖 ระบบอัตโนมัติ
• ควบคุมอุณหภูมิ: ✅ เปิด
• ควบคุมความชื้น: ✅ เปิด
• ระบบตั้งเวลา: ✅ เปิด
```

---

## ⚙️ การตั้งค่าขั้นสูง

### ปรับเปลี่ยน Cooldown Time

ในไฟล์ `main.cpp` สามารถปรับเปลี่ยนระยะเวลา Cooldown ได้:

```cpp
const unsigned long TELEGRAM_ALERT_COOLDOWN = 300000; // 5 นาที (300000 ms)
```

เปลี่ยนเป็น:
- 1 นาที: `60000`
- 10 นาที: `600000`
- 30 นาที: `1800000`

### ปรับแต่งข้อความ

สามารถแก้ไขฟังก์ชันการส่งข้อความใน `main.cpp` ได้:
- `sendTelegramSystemInfo()` - ข้อมูลระบบ
- `sendTelegramTemperatureAlert()` - แจ้งเตือนอุณหภูมิ
- `sendTelegramHumidityAlert()` - แจ้งเตือนความชื้น
- `sendTelegramWaterLevelAlert()` - แจ้งเตือนระดับน้ำ
- `sendTelegramRelayStatusAlert()` - แจ้งเตือนรีเลย์

---

## 🔍 การแก้ไขปัญหา

### ปัญหา: ไม่ได้รับข้อความแจ้งเตือน

**สาเหตุและแนวทางแก้ไข:**

1. **ตรวจสอบการตั้งค่า**
   - ✅ เปิดใช้งาน Telegram แล้วหรือยัง?
   - ✅ Bot Token ถูกต้องหรือไม่?
   - ✅ Chat ID ถูกต้องหรือไม่?

2. **ทดสอบการเชื่อมต่อ**
   - กดปุ่ม "ส่งข้อความทดสอบ" ใน Dashboard
   - ดู Serial Monitor ว่ามีข้อความ "Telegram message sent successfully" หรือไม่

3. **ตรวจสอบ WiFi**
   - ตรวจสอบว่า ESP32 เชื่อมต่อ WiFi อยู่
   - ลอง Ping IP ของ ESP32

4. **Bot Token หมดอายุ**
   - สร้าง Bot ใหม่ผ่าน @BotFather
   - อัพเดต Token ใหม่

5. **Chat ID ไม่ถูกต้อง**
   - ตรวจสอบ Chat ID อีกครั้งผ่าน @userinfobot
   - สำหรับ Group ต้องเป็นเลขติดลบ

### ปัญหา: ได้รับข้อความซ้ำมาก

**แนวทางแก้ไข:**
- เพิ่มค่า `TELEGRAM_ALERT_COOLDOWN` ให้มากขึ้น
- ปรับค่า Threshold (TEMP_FAN_ON, HUM_MAX, etc.) ให้เหมาะสม

### ปัญหา: ข้อความไม่แสดง Markdown

**แนวทางแก้ไข:**
- ตรวจสอบว่าฟังก์ชัน `sendTelegramMessage()` ใช้ `"Markdown"` เป็น parse mode
- ลองเปลี่ยนเป็น `"HTML"` แทน

---

## 📱 API Endpoints

### GET /api/telegram/config
ดึงข้อมูลการตั้งค่า Telegram

**Response:**
```json
{
  "enabled": true,
  "botToken": "123456789:ABC...",
  "chatId": "987654321",
  "alertTempHigh": true,
  "alertTempLow": true,
  "alertHumidityHigh": true,
  "alertHumidityLow": true,
  "alertWaterLevel": true,
  "alertRelayStatus": true,
  "alertSystemStartup": true
}
```

### POST /api/telegram/config
อัพเดตการตั้งค่า Telegram

**Request Body:**
```json
{
  "enabled": true,
  "botToken": "123456789:ABC...",
  "chatId": "987654321",
  "alertTempHigh": true,
  ...
}
```

### GET /api/telegram/status
ตรวจสอบสถานะ Telegram

**Response:**
```json
{
  "enabled": true,
  "configured": true,
  "botToken": "***xyz",
  "chatId": "987654321"
}
```

### POST /api/telegram/test
ส่งข้อความทดสอบ

**Response:**
```json
{
  "success": true,
  "message": "Test message sent"
}
```

---

## 🔐 ความปลอดภัย

### คำแนะนำ:
1. **ห้ามแชร์ Bot Token** - เปรียบเหมือนรหัสผ่าน
2. **ใช้ Private Chat** - ไม่ควรส่งข้อความใน Public Group
3. **เก็บ Chat ID เป็นความลับ** - ป้องกันผู้อื่นส่งข้อความปลอม
4. **อัพเดต Library** - ใช้ UniversalTelegramBot เวอร์ชันล่าสุด

---

## 📚 ข้อมูลเพิ่มเติม

### Library ที่ใช้
- **UniversalTelegramBot** v1.3.0+
- GitHub: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot

### Telegram Bot API Documentation
- https://core.telegram.org/bots/api

### การพัฒนาต่อยอด
สามารถเพิ่มฟีเจอร์ได้เช่น:
- รับคำสั่งจาก Telegram เพื่อควบคุมรีเลย์
- ส่งกราฟแสดงข้อมูล
- ตั้งค่าค่า Threshold ผ่าน Telegram
- แจ้งเตือนแบบ Schedule

---

## 🆘 ติดต่อและขอความช่วยเหลือ

หากพบปัญหาหรือต้องการความช่วยเหลือ:
1. ตรวจสอบ Serial Monitor สำหรับ Debug Messages
2. ตรวจสอบไฟล์ MQTT_TOPICS_GUIDE.md และ WEB_SERVER_GUIDE.md
3. อ่านเอกสารประกอบอื่นๆ ในโปรเจกต์

---

**สร้างโดย**: Smart Farm Control System
**เวอร์ชัน**: 1.0 (Branch: 11_AddTelegramAlert)
**วันที่**: 2026
