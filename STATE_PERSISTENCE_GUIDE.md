# 💾 State Persistence System - Smart Farm Control

## 🌟 ภาพรวม

ระบบบันทึกสถานะอัตโนมัติ (State Persistence System) ช่วยให้ Smart Farm Control สามารถกลับมาทำงานต่อได้ทันทีหลังจากไฟดับหรือบอร์ดถูกรีเซต โดยคืนค่าสถานะการทำงานทั้งหมดให้กลับมาเหมือนเดิม

---

## 🎯 ฟีเจอร์หลัก

### 1. 💾 บันทึกข้อมูลอัตโนมัติ

ระบบจะบันทึกข้อมูลต่อไปนี้อัตโนมัติ:

#### สถานะรีเลย์ (Relay States)
- ✅ สถานะพัดลม (Fan) - เปิด/ปิด
- ✅ สถานะปั๊มน้ำ (Pump) - เปิด/ปิด
- ✅ สถานะฮีตเตอร์ (Heater) - เปิด/ปิด

#### การตั้งค่าระบบ (System Settings)
- ✅ การตั้งค่าอุณหภูมิ (TEMP_FAN_ON, TEMP_FAN_OFF, TEMP_HEATER_ON, TEMP_HEATER_OFF)
- ✅ การตั้งค่าความชื้น (HUM_MIN, HUM_MAX)
- ✅ สถานะระบบอัตโนมัติ (autoTempEnabled, autoHumEnabled, scheduleEnabled)
- ✅ ตารางเวลา (Schedules) - ทั้งหมด 20 รายการ
- ✅ การตั้งค่า MQTT
- ✅ การตั้งค่า Telegram
- ✅ ชื่อเมืองสำหรับพยากรณ์อากาศ

#### ข้อมูลเซ็นเซอร์ล่าสุด (Last Sensor Readings)
- 📊 อุณหภูมิจาก DS18B20 ล่าสุด
- 📊 อุณหภูมิและความชื้นจาก XY-MD03 ล่าสุด

---

## 🔄 กลไกการทำงาน

### การบันทึก (Save)

ระบบมี 2 ไฟล์บันทึก:

#### 1. `/config.json` - การตั้งค่าหลัก
- บันทึกการตั้งค่าทั้งหมดของระบบ
- บันทึกสถานะรีเลย์ล่าสุด
- บันทึกเมื่อมีการเปลี่ยนแปลงการตั้งค่าผ่าน Dashboard

#### 2. `/state.json` - สถานะ Runtime
- บันทึกสถานะปัจจุบันของรีเลย์
- บันทึกค่าเซ็นเซอร์ล่าสุด
- บันทึกอัตโนมัติตามเงื่อนไข:
  - **ทันทีหลังเปลี่ยนสถานะรีเลย์** (debounce 2 วินาที)
  - **ทุก 30 วินาที** (หากมีการเปลี่ยนแปลง)

### ตัวอย่างการบันทึกอัตโนมัติ

```cpp
// เมื่อเปิดพัดลม
relayFan.on();
markStateChanged(); // ← ระบบจะบันทึกภายใน 2 วินาที
```

### Debounce Mechanism

เพื่อป้องกันการเขียน Flash บ่อยเกินไป:
- **Relay Change Debounce**: รอ 2 วินาทีหลังจากเปลี่ยนสถานะก่อนบันทึก
- **Periodic Save**: บันทึกทุก 30 วินาที (เฉพาะเมื่อมีการเปลี่ยนแปลง)

---

## 🚀 การคืนค่า (Restore)

เมื่อ ESP32 เริ่มต้นระบบ (boot up):

### 1. โหลดการตั้งค่าหลัก
```
Loading config from SPIFFS...
✓ Config loaded from SPIFFS
```

### 2. คืนค่าสถานะรีเลย์
```
Loading runtime state from SPIFFS...
↻ Restored Fan: OFF
↻ Restored Pump: ON
↻ Restored Heater: OFF
✓ State loaded from SPIFFS (saved at: 123456 ms)
✓ Runtime state restored
```

### 3. ระบบพร้อมใช้งาน
- รีเลย์ทุกตัวกลับมาสู่สถานะเดิม
- ระบบอัตโนมัติทำงานต่อ
- ค่าเซ็นเซอร์แสดงค่าล่าสุดทันที (จะอัพเดตเมื่ออ่านค่าใหม่)

---

## 📝 จุดที่มีการบันทึกสถานะอัตโนมัติ

### 1. Toggle Functions
```cpp
void toggleFan() {
  relayFan.toggle();
  markStateChanged(); // ← บันทึกสถานะ
}
```

### 2. Automation Functions
```cpp
void autoTemperatureControl() {
  if (temp >= TEMP_FAN_ON && !relayFan.getState()) {
    relayFan.on();
    markStateChanged(); // ← บันทึกสถานะ
  }
}
```

### 3. Schedule System
```cpp
void checkSchedules() {
  // เมื่อถึงเวลาตามตาราง
  relay->on();
  markStateChanged(); // ← บันทึกสถานะ
}
```

### 4. Web API Endpoints
```cpp
server.on("/api/relay/1/on", [](request) {
  relayFan.on();
  markStateChanged(); // ← บันทึกสถานะ
});
```

### 5. MQTT Commands
```cpp
void mqttCallback(topic, payload) {
  if (message == "ON") {
    relayFan.on();
    markStateChanged(); // ← บันทึกสถานะ
  }
}
```

---

## 🔧 การตั้งค่าขั้นสูง

### ปรับเปลี่ยน Debounce Time

ในไฟล์ [src/main.cpp](src/main.cpp):

```cpp
// รอ 2 วินาทีหลังจากเปลี่ยนสถานะก่อนบันทึก
const unsigned long RELAY_SAVE_DEBOUNCE = 2000; // ms
```

**แนะนำ:**
- เพิ่มเป็น 5000 (5 วินาที) หากมีการเปลี่ยนสถานะบ่อยมาก
- ลดเป็น 1000 (1 วินาที) หากต้องการบันทึกเร็วขึ้น

### ปรับเปลี่ยน Periodic Save Interval

```cpp
// บันทึกสถานะทุก 30 วินาที
const unsigned long STATE_SAVE_INTERVAL = 30000; // ms
```

**แนะนำ:**
- เพิ่มเป็น 60000 (1 นาที) เพื่อลดการเขียน Flash
- ลดเป็น 10000 (10 วินาที) หากต้องการความปลอดภัยสูงสุด

---

## 📊 ตัวอย่างไฟล์ที่บันทึก

### config.json
```json
{
  "tempFanOn": 30.0,
  "tempFanOff": 28.0,
  "tempHeaterOn": 20.0,
  "tempHeaterOff": 22.0,
  "humMin": 60.0,
  "humMax": 80.0,
  "autoTempEnabled": true,
  "autoHumEnabled": true,
  "scheduleEnabled": true,
  "cityName": "Nakhon Si Thammarat",
  "mqttEnabled": true,
  "mqttServer": "broker.hivemq.com",
  "mqttPort": 1883,
  "telegramEnabled": true,
  "telegramBotToken": "123456789:ABC...",
  "telegramChatId": "987654321",
  "relayFanState": false,
  "relayPumpState": true,
  "relayHeaterState": false,
  "schedules": [...]
}
```

### state.json
```json
{
  "fanState": false,
  "pumpState": true,
  "heaterState": false,
  "autoTempEnabled": true,
  "autoHumEnabled": true,
  "scheduleEnabled": true,
  "lastTemp": 25.3,
  "lastXYTemp": 26.5,
  "lastXYHum": 68.2,
  "timestamp": 1234567
}
```

---

## 🔍 การตรวจสอบผ่าน Serial Monitor

### เมื่อบันทึกสถานะ
```
✓ State saved to SPIFFS
Config saved to SPIFFS
```

### เมื่อโหลดสถานะ
```
Config loaded from SPIFFS
Restored Fan state: OFF
Restored Pump state: ON
Restored Heater state: OFF
↻ Restored Fan: OFF
↻ Restored Pump: ON
↻ Restored Heater: OFF
✓ State loaded from SPIFFS (saved at: 1234567 ms)
✓ Runtime state restored
```

### เมื่อมีการเปลี่ยนสถานะ
```
AUTO: Pump ON - Humidity 55.0% < 60.0%
✓ State saved to SPIFFS
Config saved to SPIFFS
```

---

## 🎯 สถานการณ์การใช้งาน

### สถานการณ์ 1: ไฟดับ
1. ⚡ ไฟดับขณะที่พัดลมและปั๊มกำลังเปิด
2. 💾 ระบบมีการบันทึกสถานะล่าสุดใน Flash
3. 🔌 ไฟกลับมา ESP32 รีสตาร์ท
4. ✅ ระบบโหลดสถานะและเปิดพัดลมและปั๊มกลับมาอัตโนมัติ

### สถานการณ์ 2: กดปุ่ม Reset
1. 🔘 กดปุ่ม Reset บนบอร์ด
2. 🔄 ESP32 รีสตาร์ท
3. ✅ ระบบโหลดสถานะทั้งหมดกลับมา
4. ✅ ระบบพร้อมใช้งานทันที

### สถานการณ์ 3: อัปโหลดโค้ดใหม่
1. 💻 อัปโหลดโค้ดเวอร์ชันใหม่
2. 🔄 ESP32 รีสตาร์ทหลังอัปโหลด
3. ✅ ระบบโหลดการตั้งค่าและสถานะกลับมา
4. ✅ การทำงานดำเนินต่อโดยไม่หยุดชะงัก

---

## 🛡️ ความปลอดภัยของ Flash Memory

### ป้องกันการเขียนบ่อยเกินไป

ESP32 Flash Memory มีอายุการเขียนจำกัด (~100,000 cycles)

**มาตรการป้องกัน:**
1. ✅ **Debounce**: รอ 2 วินาทีหลังเปลี่ยนสถานะ
2. ✅ **Change Detection**: บันทึกเฉพาะเมื่อมีการเปลี่ยนแปลง
3. ✅ **Periodic Save**: บันทึกเป็นระยะ (30 วิ) ไม่บ่อยเกินไป

**การประมาณอายุการใช้งาน:**
- หากบันทึก 1 ครั้ง/นาที = ~190 ปี
- หากบันทึก 1 ครั้ง/30 วินาที = ~95 ปี
- หากบันทึก 10 ครั้ง/วินาที = ~115 วัน ⚠️ (หลีกเลี่ยง!)

---

## 🔄 การทดสอบ

### การทดสอบการบันทึกและคืนค่า

1. **เปิดใช้งานรีเลย์บางตัว**
   ```
   - เปิดพัดลมผ่าน Dashboard
   - เปิดปั๊มผ่าน MQTT
   ```

2. **ตรวจสอบ Serial Monitor**
   ```
   Web: Fan ON
   ✓ State saved to SPIFFS
   ```

3. **กดปุ่ม Reset หรือตัดไฟ**

4. **ตรวจสอบ Serial Monitor อีกครั้ง**
   ```
   ↻ Restored Fan: ON
   ↻ Restored Pump: ON
   ↻ Restored Heater: OFF
   ✓ Runtime state restored
   ```

5. **ตรวจสอบ Dashboard**
   - สถานะรีเลย์ควรเหมือนก่อน Reset

---

## ⚙️ API สำหรับนักพัฒนา

### ฟังก์ชันหลัก

#### markStateChanged()
```cpp
void markStateChanged();
```
เรียกเมื่อมีการเปลี่ยนแปลงสถานะที่ต้องบันทึก

**ตัวอย่าง:**
```cpp
relayFan.on();
markStateChanged(); // ระบบจะบันทึกภายใน 2 วิ
```

#### saveStateToSPIFFS()
```cpp
void saveStateToSPIFFS();
```
บันทึกสถานะปัจจุบันลง `/state.json` ทันที

**ตัวอย่าง:**
```cpp
saveStateToSPIFFS(); // บันทึกทันที
```

#### loadStateFromSPIFFS()
```cpp
void loadStateFromSPIFFS();
```
โหลดสถานะจาก `/state.json` และคืนค่ารีเลย์

**เรียกใช้:**
- อัตโนมัติใน `setup()`
- หรือเรียกเองเมื่อต้องการโหลดใหม่

#### checkAndSaveState()
```cpp
void checkAndSaveState();
```
ตรวจสอบและบันทึกสถานะตามเงื่อนไข (debounce + periodic)

**เรียกใช้:**
- อัตโนมัติทุก loop ใน `loop()`

---

## 🐛 การแก้ไขปัญหา

### ปัญหา: สถานะไม่ถูกบันทึก

**สาเหตุและแนวทางแก้ไข:**

1. **ตรวจสอบ SPIFFS**
   ```cpp
   if (!SPIFFS.begin(true)) {
     Serial.println("SPIFFS mount failed!");
   }
   ```

2. **ตรวจสอบ Serial Monitor**
   - ควรเห็น `✓ State saved to SPIFFS`

3. **ตรวจสอบว่ามีการเรียก markStateChanged()**
   ```cpp
   relayFan.on();
   markStateChanged(); // ← อย่าลืมบรรทัดนี้!
   ```

### ปัญหา: สถานะไม่ถูกคืนค่า

**แนวทางแก้ไข:**

1. **ตรวจสอบว่ามีไฟล์ state.json**
   - ดูใน Serial Monitor: `No state file found`

2. **ลองบันทึกใหม่**
   - เปลี่ยนสถานะรีเลย์
   - รอ 2 วินาที
   - ตรวจสอบว่ามีข้อความบันทึก

3. **ลบไฟล์และสร้างใหม่**
   ```cpp
   SPIFFS.remove("/state.json");
   // จากนั้นเปลี่ยนสถานะรีเลย์ใหม่
   ```

### ปัญหา: Flash Memory เสื่อม

**อาการ:**
- ไม่สามารถบันทึกได้
- ข้อมูลหายหรือเสียหาย

**แนวทางแก้ไข:**
1. ลด frequency การบันทึก
2. เพิ่มค่า debounce และ interval
3. ใช้ External EEPROM หรือ SD Card

---

## 📚 ข้อมูลเพิ่มเติม

### ไฟล์ที่เกี่ยวข้อง
- [src/main.cpp](src/main.cpp) - โค้ดหลัก
- [platformio.ini](platformio.ini) - การตั้งค่า SPIFFS

### Filesystem
- **SPIFFS**: SPI Flash File System
- **Partition**: default.csv
- **ขนาด**: ตามไฟล์ partition table

### การพัฒนาต่อยอด

สามารถเพิ่มฟีเจอร์ได้เช่น:
- บันทึกข้อมูลลง SD Card
- ส่งข้อมูลไปยัง Cloud เป็น Backup
- สร้าง API สำหรับ Export/Import การตั้งค่า
- เพิ่มการเข้ารหัสข้อมูล

---

## 🆘 ติดต่อและขอความช่วยเหลือ

หากพบปัญหาหรือต้องการความช่วยเหลือ:
1. ตรวจสอบ Serial Monitor สำหรับ Debug Messages
2. อ่านเอกสารประกอบอื่นๆ:
   - [TELEGRAM_GUIDE.md](TELEGRAM_GUIDE.md)
   - [MQTT_TOPICS_GUIDE.md](MQTT_TOPICS_GUIDE.md)
   - [WEB_SERVER_GUIDE.md](WEB_SERVER_GUIDE.md)

---

**สร้างโดย**: Smart Farm Control System
**เวอร์ชัน**: 2.0 (Branch: 11_AddTelegramAlert)
**วันที่**: 2026
**ฟีเจอร์ใหม่**: State Persistence System - บันทึกและคืนค่าสถานะอัตโนมัติ
