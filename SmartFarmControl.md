# 📖 คู่มือระบบควบคุมฟาร์มอัจฉริยะ ESP32 Smart Farm Control System

## 📋 สารบัญ
1. [ภาพรวมของระบบ](#ภาพรวมของระบบ)
2. [สถาปัตยกรรมระบบ](#สถาปัตยกรรมระบบ)
3. [ฮาร์ดแวร์และอุปกรณ์](#ฮาร์ดแวร์และอุปกรณ์)
4. [ไลบรารีที่ใช้งาน](#ไลบรารีที่ใช้งาน)
5. [โครงสร้างโค้ดและฟังก์ชัน](#โครงสร้างโค้ดและฟังก์ชัน)
6. [ระบบควบคุมอัตโนมัติ](#ระบบควบคุมอัตโนมัติ)
7. [Web Dashboard และ API](#web-dashboard-และ-api)
8. [การติดตั้งและใช้งาน](#การติดตั้งและใช้งาน)
9. [การพัฒนาต่อยอด](#การพัฒนาต่อยอด)
10. [แนวทางแก้ไขปัญหา](#แนวทางแก้ไขปัญหา)

---

## 🌟 ภาพรวมของระบบ

### วัตถุประสงค์
ระบบควบคุมฟาร์มอัจฉริยะนี้พัฒนาขึ้นเพื่อ:
- **ตรวจวัดสภาพแวดล้อม** - วัดอุณหภูมิ, ความชื้น, คุณภาพอากาศแบบ Real-time
- **ควบคุมอุปกรณ์อัตโนมัติ** - ควบคุมพัดลม, ปั๊มน้ำ, ฮีตเตอร์ตามเงื่อนไข
- **ตั้งเวลาทำงาน** - กำหนดตารางเวลาเปิด-ปิดอุปกรณ์
- **แสดงผลและควบคุมระยะไกล** - ผ่าน OLED Display และ Web Dashboard

### คุณสมบัติหลัก
✅ **ระบบเซ็นเซอร์** - DS18B20 (อุณหภูมิ), XY-MD03 (อุณหภูมิ+ความชื้น Modbus RTU)  
✅ **ระบบควบคุม** - 3 Relays (พัดลม, ปั๊มน้ำ, ฮีตเตอร์)  
✅ **ป้องกันอันตราย** - Isolated Inputs ตรวจจับระดับน้ำ (แห้ง/ล้น)  
✅ **อินเทอร์เฟซ** - OLED Display (SSD1306), 3 Switches (Navigation)  
✅ **Web Dashboard** - Real-time monitoring และควบคุมผ่าน WiFi  
✅ **ข้อมูลสภาพอากาศ** - เชื่อมต่อ OpenWeather API  
✅ **ระบบอัตโนมัติ 3 โหมด**:
  - **Temperature Control** - ควบคุมอุณหภูมิด้วย Hysteresis
  - **Humidity Control** - ควบคุมความชื้นพร้อมตรวจสอบระดับน้ำ
  - **Schedule System** - ตั้งเวลา 10 รอบต่อวัน (NTP Sync)

---

## 🏗️ สถาปัตยกรรมระบบ

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32 DevKit V2                          │
│  ┌──────────────────────────────────────────────────────┐   │
│  │           Dual-core Xtensa 32-bit                    │   │
│  │           240MHz, 520KB RAM, 4MB Flash               │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
           │               │               │
    ┌──────┴──────┐  ┌────┴────┐  ┌───────┴────────┐
    │  Sensors    │  │ Relays  │  │   Interfaces   │
    └─────────────┘  └─────────┘  └────────────────┘
    │                │             │
    ├─ DS18B20       ├─ Relay1     ├─ OLED (I2C)
    │  (GPIO14)      │  Fan        │  - SSD1306 128x64
    │  1-Wire        │  (GPIO4)    │  - SDA: GPIO21
    │  อุณหภูมิ      │             │  - SCL: GPIO22
    │                ├─ Relay2     │
    ├─ XY-MD03       │  Pump       ├─ 3x Switches
    │  (RS485)       │  (GPIO16)   │  - SW1: GPIO34 (Enter)
    │  อุณหภูมิ+     │             │  - SW2: GPIO35 (Down)
    │  ความชื้น      ├─ Relay3     │  - SW3: GPIO32 (Up)
    │  Modbus RTU    │  Heater     │
    │  TX: GPIO1     │  (GPIO17)   ├─ WiFi Manager
    │  RX: GPIO3     │             │  - AP Mode Setup
    │                │             │  - Web Dashboard
    ├─ ISO Inputs   │             │
    │  ISO1: GPIO33  │             ├─ NTP Time Sync
    │  (Tank Dry)    │             │  - pool.ntp.org
    │  ISO2: GPIO27  │             │  - GMT+7
    │  (Overflow)    │             │
                                   └─ OpenWeather API
                                      - Weather Data
                                      - Air Quality
```

### การไหลของข้อมูล (Data Flow)

```
Sensors → ESP32 → Processing → Automation Logic → Relays
   ↓                                    ↓
OLED Display                       Web Dashboard
   ↑                                    ↑
User Input (Switches)           User Control (WiFi)
```

---

## 🔧 ฮาร์ดแวร์และอุปกรณ์

### 1. บอร์ดหลัก
| ชื่อ | รายละเอียด |
|------|-----------|
| **Microcontroller** | ESP32 DevKit V2 |
| **CPU** | Dual-core Xtensa 32-bit @ 240MHz |
| **RAM** | 520 KB SRAM |
| **Flash** | 4 MB |
| **WiFi** | 802.11 b/g/n |
| **Bluetooth** | BLE 4.2 (ไม่ได้ใช้งาน) |

### 2. เซ็นเซอร์ (Sensors)

#### DS18B20 - Digital Temperature Sensor
- **แรงดัน**: 3.0V - 5.5V
- **ช่วงวัดอุณหภูมิ**: -55°C ถึง +125°C
- **ความแม่นยำ**: ±0.5°C (-10°C ถึง +85°C)
- **โปรโตคอล**: 1-Wire (GPIO14)
- **ข้อดี**: ราคาถูก, ต่อแบบ parallel ได้หลายตัว
- **การใช้งาน**: วัดอุณหภูมิอากาศหรือดิน

#### XY-MD03 - Temperature & Humidity Sensor (Modbus RTU)
- **แรงดัน**: 5-30V DC
- **ช่วงวัดอุณหภูมิ**: -40°C ถึง +125°C (±0.5°C)
- **ช่วงวัดความชื้น**: 0-100% RH (±3% RH)
- **โปรโตคอล**: Modbus RTU ผ่าน RS485
- **Baud Rate**: 9600 (default), 8N1
- **Slave ID**: 1 (default)
- **Register**:
  - อุณหภูมิ: `0x0001` (หาร 10)
  - ความชื้น: `0x0002` (หาร 10)
- **การต่อสาย**:
  ```
  XY-MD03          ESP32 (RS485)
  VCC (Brown)   →  5-24V External
  GND (Black)   →  GND
  A (Yellow)    →  RS485 A
  B (Blue)      →  RS485 B
  ```

### 3. อุปกรณ์ควบคุม (Actuators)

#### Relays (Active Low Logic)
| Relay | GPIO | อุปกรณ์ | Logic |
|-------|------|---------|-------|
| Relay 1 | GPIO4 | **พัดลม (Fan)** | Active Low |
| Relay 2 | GPIO16 | **ปั๊มน้ำ (Pump)** | Active Low |
| Relay 3 | GPIO17 | **ฮีตเตอร์ (Heater)** | Active Low |

**หมายเหตุ**: Active Low = LOW (0V) = รีเลย์เปิด, HIGH (3.3V) = รีเลย์ปิด

### 4. Isolated Inputs (ป้องกันอันตราย)
| Input | GPIO | แรงดัน | การใช้งาน |
|-------|------|---------|----------|
| ISO1 | GPIO33 | 10-24V TTL | **ตรวจจับถังน้ำแห้ง** (Water Tank Dry) |
| ISO2 | GPIO27 | 10-24V TTL | **ตรวจจับน้ำล้น** (Water Overflow) |

**ระบบป้องกัน**: เมื่อ ISO1 Active (ถังแห้ง) → ห้ามเปิดปั๊มน้ำอัตโนมัติ

### 5. อินเทอร์เฟซผู้ใช้

#### OLED Display (SSD1306)
- **ขนาด**: 0.96" / 1.3"
- **ความละเอียด**: 128x64 pixels
- **I2C Address**: 0x3C
- **แรงดัน**: 3.3V
- **การต่อสาย**:
  ```
  OLED          ESP32
  VCC       →   3.3V
  GND       →   GND
  SDA       →   GPIO21 (I2C SDA)
  SCL       →   GPIO22 (I2C SCL)
  RST       →   GPIO5 (optional)
  ```
- **แสดงผล 3 หน้า**:
  1. **Page 0**: อุณหภูมิ DS18B20, สถานะ Relay, WiFi
  2. **Page 1**: XY-MD03 (อุณหภูมิ+ความชื้น)
  3. **Page 2**: สภาพอากาศจาก OpenWeather API

#### Switches (Active Low)
| Switch | GPIO | Pull-up | ฟังก์ชัน |
|--------|------|---------|---------|
| SW1 | GPIO34 | R10kΩ | **Enter/Select** - สลับหน้า OLED |
| SW2 | GPIO35 | R10kΩ | **Down** (สำรองไว้) |
| SW3 | GPIO32 | R10kΩ | **Up** (สำรองไว้) |

**WiFi Reset**: กด SW1 ค้างไว้ **5 วินาที** → รีเซ็ต WiFi Config

---

## 📚 ไลบรารีที่ใช้งาน

### Core Libraries (ติดตั้งผ่าน PlatformIO)

#### 1. **Adafruit GFX Library** (v1.11.3+)
```cpp
#include <Adafruit_GFX.h>
```
- **ไลบรารีกราฟิก** สำหรับวาดรูป, ข้อความ, เส้น บนจอ OLED
- **ฟังก์ชันหลัก**:
  - `drawPixel()` - วาดจุด
  - `drawLine()` - วาดเส้น
  - `drawRect()` - วาดกรอบสี่เหลี่ยม
  - `print()` / `println()` - แสดงข้อความ
  - `setTextSize()` / `setTextColor()` - ตั้งค่าข้อความ
- **การใช้งาน**: ใช้ร่วมกับ Adafruit_SSD1306

#### 2. **Adafruit SSD1306** (v2.5.7+)
```cpp
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, &Wire, -1);
```
- **ไดรเวอร์จอ OLED** รุ่น SSD1306 (128x64)
- **ฟังก์ชันหลัก**:
  - `begin(SSD1306_SWITCHCAPVCC, 0x3C)` - เริ่มต้นจอ
  - `clearDisplay()` - ลบหน้าจอ
  - `display()` - อัปเดตหน้าจอ (บังคับแสดงผล)
  - `setCursor(x, y)` - ตำแหน่งเคอร์เซอร์
- **การใช้งาน**: แสดงผลข้อมูลเซ็นเซอร์, เมนู, สถานะ

#### 3. **WiFiManager** (v2.0.17)
```cpp
#include <WiFiManager.h>
WiFiManager wifiManager;
```
- **จัดการ WiFi แบบอัตโนมัติ** ไม่ต้อง hard-code SSID/Password
- **ฟีเจอร์**:
  - สแกนและแสดงรายชื่อ WiFi ในบริเวณ
  - หน้า Web Portal สำหรับกรอก SSID/Password
  - บันทึกค่าลง EEPROM (ใช้ครั้งเดียว)
  - รีเซ็ตการตั้งค่าได้ (กด SW1 ค้าง 5 วิ)
- **การใช้งาน**:
  ```cpp
  wifiManager.autoConnect("ESP32-SmartFarm");
  // จะสร้าง AP: "ESP32-SmartFarm" ถ้ายังไม่เชื่อม WiFi
  ```

#### 4. **OneWire** (v2.3.7+) + **DallasTemperature** (v3.9.0+)
```cpp
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(14);  // GPIO14
DallasTemperature tempSensor(&oneWire);
```
- **โปรโตคอล 1-Wire** สำหรับเซ็นเซอร์ DS18B20
- **ฟังก์ชันหลัก**:
  - `begin()` - เริ่มต้น
  - `requestTemperatures()` - สั่งอ่านอุณหภูมิ
  - `getTempCByIndex(0)` - ดึงค่าอุณหภูมิ (°C)
- **การใช้งาน**: อ่านอุณหภูมิอากาศ/น้ำ/ดิน

#### 5. **ModbusMaster** (v2.0.1)
```cpp
#include <ModbusMaster.h>
ModbusMaster modbus;
modbus.begin(1, Serial);  // SlaveID=1
```
- **โปรโตคอล Modbus RTU** สื่อสารกับ XY-MD03
- **ฟังก์ชันหลัก**:
  - `readInputRegisters(address, qty)` - อ่าน Input Register
  - `getResponseBuffer(index)` - ดึงค่าจาก Response
- **การใช้งาน**:
  ```cpp
  modbus.readInputRegisters(0x0001, 2);  // อ่าน Temp+Hum
  float temp = modbus.getResponseBuffer(0) / 10.0;
  float hum = modbus.getResponseBuffer(1) / 10.0;
  ```

#### 6. **ArduinoJson** (v7.4.3)
```cpp
#include <ArduinoJson.h>
JsonDocument doc;
```
- **จัดการ JSON** สำหรับ API และ Web
- **ฟังก์ชันหลัก**:
  - `doc["key"] = value` - เพิ่มข้อมูล
  - `serializeJson(doc, output)` - แปลงเป็น JSON String
  - `deserializeJson(doc, input)` - แปลงจาก JSON String
- **การใช้งาน**: 
  - ส่งข้อมูลเซ็นเซอร์เป็น JSON ไปยัง Web
  - รับการตั้งค่าจาก Web (POST /api/config)
  - บันทึก/โหลด config ลง SPIFFS

#### 7. **ESPAsyncWebServer** + **AsyncTCP**
```cpp
#include <ESPAsyncWebServer.h>
AsyncWebServer server(80);
```
- **Web Server แบบ Asynchronous** (ไม่บล็อก main loop)
- **ฟังก์ชันหลัก**:
  - `server.on(path, method, handler)` - กำหนด Route
  - `request->send(code, contentType, body)` - ส่ง Response
  - `request->send(SPIFFS, "/index.html")` - ส่งไฟล์จาก SPIFFS
- **การใช้งาน**:
  ```cpp
  server.on("/api/sensors", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send(200, "application/json", getSensorDataJSON());
  });
  ```

#### 8. **HTTPClient**
```cpp
#include <HTTPClient.h>
HTTPClient http;
```
- **HTTP Client** เรียก REST API ภายนอก
- **การใช้งาน**:
  ```cpp
  http.begin("https://api.openweathermap.org/...");
  int httpCode = http.GET();
  String payload = http.getString();
  ```

#### 9. **SPIFFS**
```cpp
#include <SPIFFS.h>
```
- **File System** เก็บไฟล์ HTML/CSS/JS/Config
- **ฟังก์ชันหลัก**:
  - `SPIFFS.begin()` - เริ่มต้น
  - `SPIFFS.open(path, mode)` - เปิดไฟล์
  - `file.read()` / `file.write()` - อ่าน/เขียน
- **การใช้งาน**:
  - `/index.html` - หน้า Dashboard
  - `/settings.html` - หน้าตั้งค่า
  - `/config.json` - บันทึกค่า Automation

### Custom Classes (โฟลเดอร์ `include/`)

#### DevSwitch.h
```cpp
class DevSwitch {
  void begin();
  bool isPressed();
  void setCallback(CallbackFunction func);
  void update();  // วนซ้ำใน loop()
};
```
- **จัดการปุ่มกด** พร้อม Debouncing
- **รองรับ**: Single Click, Long Press
- **การใช้งาน**: นำทาง OLED, รีเซ็ต WiFi

#### DevRelay.h
```cpp
class DevRelayWithTimer {
  void on();
  void off();
  void toggle();
  bool getState();
  void update();  // วนซ้ำใน loop()
};
```
- **ควบคุม Relay** พร้อม Timer (อนาคต)
- **รองรับ**: Active High/Low Logic
- **การใช้งาน**: เปิด-ปิด พัดลม/ปั๊ม/ฮีตเตอร์

#### DevIsoInput.h
```cpp
class DevIsoInput {
  bool isActive();
  void setOnActiveCallback(CallbackFunction func);
  void update();  // วนซ้ำใน loop()
};
```
- **อ่านสัญญาณ Isolated Input** (10-24V)
- **รองรับ**: Callback เมื่อสถานะเปลี่ยน
- **การใช้งาน**: ตรวจจับระดับน้ำ, เซ็นเซอร์ดิจิทัล

#### DevTempHumidity.h
```cpp
class DevTempHumidity {
  void begin(baudRate);
  bool read();
  float getTemperature();
  float getHumidity();
  bool isConnected();
};
```
- **อ่านค่า XY-MD03** ผ่าน Modbus RTU
- **Auto Direction**: ไม่ต้องต่อ DE/RE pin
- **การใช้งาน**: วัดอุณหภูมิและความชื้นอากาศ

---

## 🧩 โครงสร้างโค้ดและฟังก์ชัน

### โครงสร้างไฟล์โปรเจกต์
```
ESP32_DEV_SMARTFARM_CONTROL/
├── platformio.ini          # การตั้งค่า PlatformIO
├── SmartFarmControl.md     # เอกสารนี้
├── HardwareESP32Config.md  # คู่มือฮาร์ดแวร์
├── src/
│   └── main.cpp            # โค้ดหลัก (1537 lines)
├── include/
│   ├── DevSwitch.h         # คลาสจัดการปุ่มกด
│   ├── DevRelay.h          # คลาสควบคุม Relay
│   ├── DevIsoInput.h       # คลาสอ่าน Isolated Input
│   └── DevTempHumidity.h   # คลาสอ่าน XY-MD03 Modbus
└── data/                   # ไฟล์ SPIFFS (อัปโหลดแยก)
    ├── index.html          # หน้า Dashboard (~550 lines)
    ├── settings.html       # หน้าตั้งค่า (~600 lines)
    └── config.json         # การตั้งค่า Automation (สร้างอัตโนมัติ)
```

### ส่วนประกอบหลักใน main.cpp

#### 1. การตั้งค่าเริ่มต้น (Lines 1-150)
```cpp
// ประกาศตัวแปร Global
float TEMP_FAN_ON = 30.0;     // ค่า threshold
bool autoTempEnabled = false; // สถานะเปิด/ปิด
Schedule schedules[10];       // ตารางเวลา 10 รอบ

// สร้าง Objects
DevSwitch sw1(PIN_SW1);
DevRelayWithTimer relayFan(PIN_RELAY1, true);
OneWire oneWire(PIN_TEMP_SENSOR);
```

#### 2. ฟังก์ชัน `setup()` (Lines 1400-1450)
```cpp
void setup() {
  Serial.begin(115200);
  
  // เริ่มต้น Hardware
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  tempSensor.begin();
  xymd03.begin(9600);
  SPIFFS.begin();
  
  // เริ่มต้น I/O
  sw1.begin();
  relayFan.begin();
  iso1.begin();
  
  // เชื่อม WiFi
  setupWiFi();
  
  // โหลดการตั้งค่า
  loadConfigFromSPIFFS();
  
  // ตั้งเวลา NTP
  configTime(7*3600, 0, "pool.ntp.org");
  
  // เริ่ม Web Server
  setupWebServer();
  server.begin();
  
  // แสดงหน้าจอเริ่มต้น
  showWelcome();
}
```

#### 3. ฟังก์ชัน `loop()` (Lines 1470-1510)
```cpp
void loop() {
  unsigned long now = millis();
  
  // อัปเดต Switches & Inputs (ทุก loop)
  sw1.update();
  sw2.update();
  sw3.update();
  iso1.update();
  iso2.update();
  relayFan.update();
  relayPump.update();
  relayHeater.update();
  
  // ตรวจสอบ WiFi Reset (กด SW1 5 วิ)
  checkWiFiResetButton();
  
  // อ่านค่าเซ็นเซอร์ (ทุก 2-3 วิ)
  if (now - lastTempUpdate >= TEMP_UPDATE_INTERVAL) {
    readTemperature();      // DS18B20
    lastTempUpdate = now;
  }
  
  if (now - lastXYMD03Update >= XYMD03_UPDATE_INTERVAL) {
    readXYMD03();           // XY-MD03
    lastXYMD03Update = now;
  }
  
  // อัปเดต Display (ทุก 250ms)
  if (now - lastDisplayUpdate >= DISPLAY_INTERVAL) {
    updateDisplay();
    lastDisplayUpdate = now;
  }
  
  // ดึงข้อมูลสภาพอากาศ (ทุก 10 นาที)
  if (now - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL) {
    fetchWeatherData();
    fetchAirQualityData();
    lastWeatherUpdate = now;
  }
  
  // ระบบควบคุมอัตโนมัติ (ทุก 5 วิ)
  if (now - lastAutoCheck >= AUTO_CHECK_INTERVAL) {
    autoWaterLevelControl();  // ตรวจสอบระดับน้ำก่อน
    
    if (autoTempEnabled) {
      autoTemperatureControl();
    }
    
    if (autoHumEnabled) {
      autoHumidityControl();
    }
    
    if (scheduleEnabled) {
      checkSchedules();
    }
    
    lastAutoCheck = now;
  }
}
```

#### 4. ฟังก์ชันจัดการเซ็นเซอร์

**readTemperature() - อ่านค่า DS18B20**
```cpp
void readTemperature() {
  tempSensor.requestTemperatures();
  float temp = tempSensor.getTempCByIndex(0);
  
  if (temp != DEVICE_DISCONNECTED_C) {
    currentTemperature = temp;
    sensorConnected = true;
  } else {
    sensorConnected = false;
    simulateTemperature();  // ใช้ค่าจำลองถ้าไม่ต่อเซ็นเซอร์
  }
}
```

**readXYMD03() - อ่านค่า XY-MD03 Modbus**
```cpp
void readXYMD03() {
  if (xymd03.read()) {
    xymd03_temperature = xymd03.getTemperature();
    xymd03_humidity = xymd03.getHumidity();
    xymd03_connected = true;
  } else {
    xymd03_connected = false;
    simulateXYMD03();  // ใช้ค่าจำลอง
  }
}
```

**fetchWeatherData() - ดึงข้อมูลจาก OpenWeather API**
```cpp
void fetchWeatherData() {
  String url = "http://api.openweathermap.org/data/2.5/weather?q=";
  url += cityName;
  url += "&appid=" + OPENWEATHER_API_KEY;
  url += "&units=metric&lang=th";
  
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    JsonDocument doc;
    deserializeJson(doc, payload);
    
    weather_temp = doc["main"]["temp"];
    weather_humidity = doc["main"]["humidity"];
    weather_description = doc["weather"][0]["description"].as<String>();
    weather_lat = doc["coord"]["lat"];
    weather_lon = doc["coord"]["lon"];
    
    weatherDataAvailable = true;
  }
  http.end();
}
```

---

## 🤖 ระบบควบคุมอัตโนมัติ

### 1. Temperature Control (ควบคุมอุณหภูมิ)

**หนักการทำงาน**: ใช้แนวคิด **Hysteresis** เพื่อป้องกันการเปิด-ปิดบ่อย

```cpp
void autoTemperatureControl() {
  if (!autoTempEnabled) return;
  
  float temp = currentTemperature;  // อ่านจาก DS18B20
  
  // ========== พัดลม (ระบายความร้อน) ==========
  if (temp >= TEMP_FAN_ON && !relayFan.getState()) {
    relayFan.on();  // เปิดพัดลมเมื่ออุณหภูมิ ≥ 30°C
  } 
  else if (temp <= TEMP_FAN_OFF && relayFan.getState()) {
    relayFan.off(); // ปิดพัดลมเมื่ออุณหภูมิ ≤ 28°C
  }
  
  // ========== ฮีตเตอร์ (เพิ่มความร้อน) ==========
  if (temp <= TEMP_HEATER_ON && !relayHeater.getState()) {
    relayHeater.on();  // เปิดฮีตเตอร์เมื่ออุณหภูมิ ≤ 20°C
  } 
  else if (temp >= TEMP_HEATER_OFF && relayHeater.getState()) {
    relayHeater.off(); // ปิดฮีตเตอร์เมื่ออุณหภูมิ ≥ 22°C
  }
}
```

**การตั้งค่า Default**:
- `TEMP_FAN_ON = 30.0°C` → เปิดพัดลม
- `TEMP_FAN_OFF = 28.0°C` → ปิดพัดลม (Hysteresis 2°C)
- `TEMP_HEATER_ON = 20.0°C` → เปิดฮีตเตอร์
- `TEMP_HEATER_OFF = 22.0°C` → ปิดฮีตเตอร์ (Hysteresis 2°C)

**ตัวอย่างการทำงาน**:
```
อุณหภูมิ:  28°C → 29°C → 30°C → 31°C → 30°C → 29°C → 28°C → 27°C
พัดลม:      OFF    OFF    ON     ON     ON     ON     OFF    OFF
           (รอถึง 30°)  (เปิด)        (คงสถานะ)    (ปิดที่ 28°)
```

### 2. Humidity Control (ควบคุมความชื้น)

**หลักการทำงาน**: ควบคุมความชื้นพร้อมตรวจสอบระดับน้ำเพื่อความปลอดภัย

```cpp
void autoHumidityControl() {
  if (!autoHumEnabled) return;
  
  float humidity = xymd03_humidity;  // อ่านจาก XY-MD03
  
  // ========== ปั๊มน้ำ (เพิ่มความชื้น) ==========
  if (humidity < HUM_MIN && !relayPump.getState()) {
    // ⚠️ ตรวจสอบระดับน้ำก่อน (Safety First!)
    if (!iso1.isActive()) {  // ISO1 = Tank Dry Sensor
      relayPump.on();  // เปิดปั๊มเมื่อความชื้น < 60%
    } else {
      Serial.println("⚠️ ไม่สามารถเปิดปั๊มได้ - ถังน้ำแห้ง!");
    }
  }
  
  // ========== พัดลม (ลดความชื้น) ==========
  if (humidity > HUM_MAX && !relayFan.getState()) {
    relayFan.on();  // เปิดพัดลมเมื่อความชื้น > 80%
  }
  else if (humidity <= HUM_MAX - 5 && relayFan.getState()) {
    relayFan.off(); // ปิดพัดลมเมื่อความชื้น ≤ 75%
  }
}
```

**การตั้งค่า Default**:
- `HUM_MIN = 60.0%` → เปิดปั๊มน้ำ (พ่นหมอก)
- `HUM_MAX = 80.0%` → เปิดพัดลม (ระบายความชื้น)

**ระบบป้องกัน**:
```cpp
void autoWaterLevelControl() {
  // ถ้าถังแห้ง → ปิดปั๊มทันที
  if (iso1.isActive() && relayPump.getState()) {
    relayPump.off();
    Serial.println("⚠️ ปิดปั๊มฉุกเฉิน - ถังน้ำแห้ง!");
  }
  
  // ถ้าน้ำล้น → ปิดปั๊มทันที
  if (iso2.isActive() && relayPump.getState()) {
    relayPump.off();
    Serial.println("⚠️ ปิดปั๊มฉุกเฉิน - น้ำล้น!");
  }
}
```

### 3. Schedule System (ตั้งเวลาทำงาน)

**หลักการทำงาน**: ตรวจสอบเวลาจาก **NTP** และควบคุม Relay ตามตาราง

```cpp
void checkSchedules() {
  if (!scheduleEnabled) return;
  
  // ดึงเวลาปัจจุบันจาก NTP
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  
  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;
  
  // ป้องกันการทำงานซ้ำในนาทีเดียวกัน
  if (currentMinute == lastScheduleMinute) return;
  lastScheduleMinute = currentMinute;
  
  // ตรวจสอบตารางเวลาทั้ง 10 รอบ
  for (int i = 0; i < 10; i++) {
    if (!schedules[i].enabled) continue;
    
    // ถ้าตรงเวลา → ทำงาน
    if (schedules[i].hour == currentHour && 
        schedules[i].minute == currentMinute) {
      
      int relay = schedules[i].relayNum;
      bool turnOn = schedules[i].turnOn;
      
      if (relay == 1) {  // Fan
        turnOn ? relayFan.on() : relayFan.off();
      }
      else if (relay == 2) {  // Pump
        turnOn ? relayPump.on() : relayPump.off();
      }
      else if (relay == 3) {  // Heater
        turnOn ? relayHeater.on() : relayHeater.off();
      }
      
      Serial.printf("⏰ Schedule #%d: Relay%d → %s\n", 
                    i, relay, turnOn ? "ON" : "OFF");
    }
  }
}
```

**โครงสร้าง Schedule**:
```cpp
struct Schedule {
  uint8_t hour;       // 0-23
  uint8_t minute;     // 0-59
  uint8_t relayNum;   // 1=Fan, 2=Pump, 3=Heater
  bool turnOn;        // true=เปิด, false=ปิด
  bool enabled;       // true=ใช้งาน
};
```

**ตัวอย่างการตั้งค่า**:
```cpp
Schedule schedules[10] = {
  {6, 0, 2, true, true},   // 06:00 เปิดปั๊มน้ำ
  {6, 30, 2, false, true}, // 06:30 ปิดปั๊มน้ำ
  {12, 0, 1, true, true},  // 12:00 เปิดพัดลม
  {18, 0, 1, false, true}, // 18:00 ปิดพัดลม
  {20, 0, 3, true, true},  // 20:00 เปิดฮีตเตอร์
  {7, 0, 3, false, true},  // 07:00 ปิดฮีตเตอร์
  ...
};
```

### การบันทึกและโหลดการตั้งค่า (SPIFFS)

**saveConfigToSPIFFS()**:
```cpp
void saveConfigToSPIFFS() {
  File file = SPIFFS.open("/config.json", "w");
  
  JsonDocument doc;
  doc["tempFanOn"] = TEMP_FAN_ON;
  doc["tempFanOff"] = TEMP_FAN_OFF;
  doc["humMin"] = HUM_MIN;
  doc["humMax"] = HUM_MAX;
  doc["autoTempEnabled"] = autoTempEnabled;
  doc["scheduleEnabled"] = scheduleEnabled;
  
  JsonArray schedArray = doc["schedules"].to<JsonArray>();
  for (int i = 0; i < 10; i++) {
    schedArray[i]["hour"] = schedules[i].hour;
    schedArray[i]["minute"] = schedules[i].minute;
    schedArray[i]["relayNum"] = schedules[i].relayNum;
    schedArray[i]["turnOn"] = schedules[i].turnOn;
    schedArray[i]["enabled"] = schedules[i].enabled;
  }
  
  serializeJson(doc, file);
  file.close();
}
```

**loadConfigFromSPIFFS()**:
```cpp
void loadConfigFromSPIFFS() {
  File file = SPIFFS.open("/config.json", "r");
  if (!file) return;
  
  JsonDocument doc;
  deserializeJson(doc, file);
  
  TEMP_FAN_ON = doc["tempFanOn"] | TEMP_FAN_ON;
  HUM_MIN = doc["humMin"] | HUM_MIN;
  autoTempEnabled = doc["autoTempEnabled"] | false;
  
  JsonArray schedArray = doc["schedules"];
  for (int i = 0; i < 10; i++) {
    schedules[i].hour = schedArray[i]["hour"];
    schedules[i].enabled = schedArray[i]["enabled"];
  }
  
  file.close();
}
```

---

## 🌐 Web Dashboard และ API

### หน้าเว็บ (SPIFFS Files)

#### 1. **index.html** - หน้า Dashboard หลัก
**URL**: `http://[ESP32_IP]/`

**ฟีเจอร์**:
- แสดงข้อมูลเซ็นเซอร์ Real-time (อัปเดตทุก 2 วิ)
  - DS18B20 อุณหภูมิ
  - XY-MD03 อุณหภูมิ + ความชื้น
  - สถานะ ISO Inputs (ระดับน้ำ)
  - ข้อมูลสภาพอากาศ (OpenWeather)
- ควบคุม Relay (เปิด/ปิด) ด้วยปุ่มกด
- แสดงสถานะการเชื่อมต่อ WiFi, IP Address

**เทคโนโลยี**:
- HTML5 + CSS3 (Gradient Design)
- JavaScript (Fetch API, Auto-refresh)
- Responsive Design (รองรับมือถือ)

**โครงสร้าง**:
```html
<!DOCTYPE html>
<html lang="th">
<head>
  <style>
    /* ไล่สีม่วง-ฟ้า */
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  </style>
</head>
<body>
  <div class="container">
    <!-- ปุ่ม Settings -->
    <a href="/settings" class="settings-btn">⚙️ Settings</a>
    
    <!-- หัวข้อ -->
    <h1>🌱 ESP32 Smart Farm Control</h1>
    <p>IP: <span id="ipAddress"></span></p>
    
    <!-- Cards แสดงข้อมูล -->
    <div class="grid">
      <div class="card"><!-- DS18B20 --></div>
      <div class="card"><!-- XY-MD03 --></div>
      <div class="card"><!-- Relay Controls --></div>
      <div class="card"><!-- Weather --></div>
    </div>
  </div>
  
  <script>
    // Auto-refresh ทุก 2 วิ
    setInterval(() => {
      fetchSensors();
      fetchRelays();
    }, 2000);
    
    function fetchSensors() {
      fetch('/api/sensors')
        .then(res => res.json())
        .then(data => {
          document.getElementById('temp').innerText = data.ds18b20.temperature;
          // ...
        });
    }
  </script>
</body>
</html>
```

#### 2. **settings.html** - หน้าตั้งค่า Automation
**URL**: `http://[ESP32_IP]/settings`

**ฟีเจอร์**:
- **Automation Control** - เปิด/ปิด ระบบควบคุมอัตโนมัติ 3 โหมด
  - Temperature Control (พัดลม/ฮีตเตอร์)
  - Humidity Control (ปั๊มน้ำ/พัดลม)
  - Schedule System (ตารางเวลา)
- **Temperature Settings** - ตั้งค่า threshold
  - Fan ON/OFF (°C)
  - Heater ON/OFF (°C)
- **Humidity Settings** - ตั้งค่า threshold
  - Min/Max (%)
- **Weather Location** - เปลี่ยนเมือง OpenWeather
- **Schedule System** - จัดการตารางเวลา 10 รอบ
  - เพิ่ม/ลบ/แก้ไข
  - เลือก Relay (Fan/Pump/Heater)
  - เลือกเวลา (Hour:Minute)
  - เลือกการทำงาน (เปิด/ปิด)

**การบันทึก**:
```javascript
function saveSettings() {
  const config = {
    tempFanOn: parseFloat(document.getElementById('tempFanOn').value),
    humMin: parseFloat(document.getElementById('humMin').value),
    autoTempEnabled: document.getElementById('autoTempEnabled').checked,
    schedules: [
      {hour: 6, minute: 0, relayNum: 2, turnOn: true, enabled: true},
      // ...
    ]
  };
  
  fetch('/api/config', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify(config)
  })
  .then(res => res.json())
  .then(data => {
    if (data.success) {
      alert('✅ บันทึกสำเร็จ!');
    }
  });
}
```

### REST API Endpoints

#### GET `/api/sensors` - ดึงข้อมูลเซ็นเซอร์
**Response**:
```json
{
  "ds18b20": {
    "temperature": 26.5,
    "connected": true
  },
  "xymd03": {
    "temperature": 27.2,
    "humidity": 68.5,
    "connected": true
  },
  "iso1": {
    "active": false,
    "description": "Tank Water Level (Dry)"
  },
  "iso2": {
    "active": false,
    "description": "Tank Water Level (Overflow)"
  },
  "weather": {
    "temperature": 29.0,
    "humidity": 75,
    "description": "เมฆเล็กน้อย",
    "aqi": 2,
    "pm2_5": 15.2
  }
}
```

#### GET `/api/relay/{n}/{action}` - ควบคุม Relay
**Parameters**:
- `n`: 1 (Fan), 2 (Pump), 3 (Heater)
- `action`: `on` หรือ `off`

**ตัวอย่าง**:
```
GET /api/relay/1/on   → เปิดพัดลม
GET /api/relay/2/off  → ปิดปั๊มน้ำ
```

**Response**:
```json
{
  "relay": 1,
  "state": true,
  "name": "Fan"
}
```

#### GET `/api/config` - ดึงการตั้งค่า Automation
**Response**:
```json
{
  "tempFanOn": 30.0,
  "tempFanOff": 28.0,
  "humMin": 60.0,
  "humMax": 80.0,
  "autoTempEnabled": true,
  "autoHumEnabled": false,
  "scheduleEnabled": true,
  "cityName": "Nakhon Si Thammarat",
  "schedules": [
    {
      "hour": 6,
      "minute": 0,
      "relayNum": 2,
      "turnOn": true,
      "enabled": true
    }
  ]
}
```

#### POST `/api/config` - บันทึกการตั้งค่า
**Request Body**: (JSON เหมือน GET `/api/config`)

**Response**:
```json
{
  "success": true
}
```

**การทำงาน**:
1. รับค่าจาก Web
2. อัปเดตตัวแปร Global
3. บันทึกลง SPIFFS (`/config.json`)
4. ตอบกลับ `success: true`

#### GET `/api/automation/status` - สถานะ Automation Real-time
**Response**:
```json
{
  "autoTempEnabled": true,
  "autoHumEnabled": false,
  "scheduleEnabled": true,
  "currentTemp": 26.5,
  "currentHumidity": 68.5,
  "relays": {
    "fan": false,
    "pump": false,
    "heater": false
  },
  "waterLevel": {
    "dry": false,
    "overflow": false
  }
}
```

### การตั้งค่า Web Server (setupWebServer)
```cpp
void setupWebServer() {
  // Enable CORS (สำหรับ Cross-Origin)
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  
  // หน้าเว็บ
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(SPIFFS, "/index.html", "text/html");
  });
  
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(SPIFFS, "/settings.html", "text/html");
  });
  
  // API Endpoints
  server.on("/api/sensors", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "application/json", getSensorDataJSON());
  });
  
  server.on("/api/relay/1/on", HTTP_GET, [](AsyncWebServerRequest *req) {
    relayFan.on();
    req->send(200, "application/json", getRelayStatusJSON());
  });
  
  server.onBody([](AsyncWebServerRequest *req, uint8_t *data, size_t len) {
    if (req->url() == "/api/config" && req->method() == HTTP_POST) {
      JsonDocument doc;
      deserializeJson(doc, (char*)data);
      
      if (!doc["tempFanOn"].isNull()) {
        TEMP_FAN_ON = doc["tempFanOn"].as<float>();
      }
      
      saveConfigToSPIFFS();
      req->send(200, "application/json", "{\"success\":true}");
    }
  });
}
```

---

## 🚀 การติดตั้งและใช้งาน

### ขั้นตอนที่ 1: ติดตั้ง Software Tools

#### 1.1 ติดตั้ง VS Code
- ดาวน์โหลด: https://code.visualstudio.com/

#### 1.2 ติดตั้ง PlatformIO Extension
```
VS Code → Extensions (Ctrl+Shift+X) → ค้นหา "PlatformIO IDE" → Install
```

#### 1.3 ติดตั้ง USB Driver สำหรับ ESP32
- **Windows**: ดาวน์โหลด CP2102/CH340 Driver
- **Linux/Mac**: ใช้ได้เลยในระบบปกติ

### ขั้นตอนที่ 2: Clone โปรเจกต์

```bash
git clone https://github.com/thaitechzone/ESP32_DEV_SMARTFARM_CONTROL.git
cd ESP32_DEV_SMARTFARM_CONTROL
```

### ขั้นตอนที่ 3: เปิดโปรเจกต์ใน PlatformIO

```
VS Code → File → Open Folder → เลือกโฟลเดอร์โปรเจกต์
PlatformIO จะติดตั้ง Libraries อัตโนมัติตาม platformio.ini
```

### ขั้นตอนที่ 4: ต่อสายฮาร์ดแวร์

#### ต่อสาย DS18B20
```
DS18B20    →  ESP32
VCC        →  3.3V
GND        →  GND
DATA       →  GPIO14 (ต่อ Pull-up 4.7kΩ ไป 3.3V)
```

#### ต่อสาย XY-MD03 (RS485)
```
XY-MD03         →  RS485 Module  →  ESP32
VCC (Brown)     →  5-24V External
GND (Black)     →  GND
A (Yellow)      →  A
B (Blue)        →  B
                   RO            →  GPIO3 (RX)
                   DI            →  GPIO1 (TX)
```

#### ต่อสาย OLED Display
```
OLED    →  ESP32
VCC     →  3.3V
GND     →  GND
SDA     →  GPIO21
SCL     →  GPIO22
```

#### ต่อสาย Relays
```
Relay Module  →  ESP32       →  อุปกรณ์
IN1           →  GPIO4       →  พัดลม (Fan)
IN2           →  GPIO16      →  ปั๊มน้ำ (Pump)
IN3           →  GPIO17      →  ฮีตเตอร์ (Heater)
VCC           →  5V
GND           →  GND
COM           →  220V AC or 12V DC
NO/NC         →  โหลด
```

### ขั้นตอนที่ 5: Compile และ Upload

#### 5.1 Compile โค้ด
```
PlatformIO → Project Tasks → esp32doit-devkit-v1 → General → Build
หรือกด: Ctrl+Alt+B
```

#### 5.2 Upload SPIFFS (ไฟล์ Web)
```
PlatformIO → Project Tasks → Platform → Upload Filesystem Image
หรือเทอร์มินัล: pio run --target uploadfs
```

#### 5.3 Upload Firmware
```
PlatformIO → Project Tasks → General → Upload
หรือกด: Ctrl+Alt+U
```

### ขั้นตอนที่ 6: ตั้งค่า WiFi

#### 6.1 Boot ครั้งแรก
```
ESP32 จะสร้าง Access Point: "ESP32-SmartFarm"
```

#### 6.2 เชื่อมต่อ WiFi
```
1. เชื่อมมือถือเข้า WiFi: ESP32-SmartFarm
2. เปิดเบราว์เซอร์ไปที่: 192.168.4.1
3. เลือก WiFi ที่ต้องการ → กรอกรหัสผ่าน → Save
4. ESP32 จะรีสตาร์ทและเชื่อมต่อ WiFi อัตโนมัติ
```

#### 6.3 หา IP Address ของ ESP32
```
- เปิด Serial Monitor (115200 baud)
- หรือดู OLED Display (หน้าจอ Page 0)
```

### ขั้นตอนที่ 7: เปิดใช้งาน Web Dashboard

```
1. เปิดเบราว์เซอร์ไปที่: http://[ESP32_IP]/
2. จะเห็นหน้า Dashboard แสดงข้อมูลเซ็นเซอร์
3. กดปุ่ม ⚙️ Settings เพื่อตั้งค่า Automation
4. กำหนดค่า Threshold และ Schedule
5. กด "Save Settings"
```

### ขั้นตอนที่ 8: ทดสอบการทำงาน

#### ทดสอบ Manual Control
```
1. หน้า Dashboard → กดปุ่ม "ON" ที่ Fan/Pump/Heater
2. ตรวจสอบว่า Relay ทำงาน (ไฟ LED บนโมดูล Relay ติด)
3. ตรวจสอบ Serial Monitor เห็นข้อความ "Fan: ON"
```

#### ทดสอบ Temperature Control
```
1. หน้า Settings → เปิด "Temperature Control"
2. ตั้ง "Fan ON" = 25°C (ต่ำกว่าอุณหภูมิปัจจุบัน)
3. Save Settings
4. รอ 5 วิ → พัดลมจะเปิดอัตโนมัติ
5. ตั้ง "Fan OFF" = 30°C → พัดลมจะปิด
```

#### ทดสอบ Schedule
```
1. หน้า Settings → เปิด "Schedule System"
2. กด "Add Schedule"
3. ตั้งเวลา: 14:30, Relay: Fan, Action: ON
4. Save Settings
5. รอถึงเวลา 14:30 → พัดลมจะเปิดอัตโนมัติ
```

### ขั้นตอนที่ 9: รีเซ็ต WiFi (ถ้าจำเป็น)

```
1. กดปุ่ม SW1 (GPIO34) ค้างไว้ 5 วินาที
2. OLED จะแสดง Countdown 5, 4, 3, 2, 1
3. WiFi Settings จะถูกลบ
4. ESP32 รีสตาร์ท → กลับไปเป็น AP Mode
```

---

## 💡 การพัฒนาต่อยอด

### 1. เพิ่ม Sensors และ I/O

#### เพิ่มเซ็นเซอร์ดิน (Soil Moisture)
```cpp
// เพิ่มในส่วน Declarations
const uint8_t PIN_SOIL_MOISTURE = 36;  // ADC1_CH0

void readSoilMoisture() {
  int raw = analogRead(PIN_SOIL_MOISTURE);
  float moisture = map(raw, 0, 4095, 0, 100);  // แปลงเป็น %
  
  // เพิ่มเข้า JSON
  doc["soilMoisture"] = moisture;
}
```

#### เพิ่มเซ็นเซอร์แสง (Light Sensor LDR)
```cpp
const uint8_t PIN_LDR = 39;  // ADC1_CH3

void readLightLevel() {
  int raw = analogRead(PIN_LDR);
  float lux = map(raw, 0, 4095, 0, 1000);  // ประมาณการ lux
  
  // ควบคุมไฟ LED อัตโนมัติ
  if (lux < 100) {  // มืด
    digitalWrite(PIN_LED, HIGH);  // เปิดไฟ
  }
}
```

#### เพิ่มเซ็นเซอร์ pH (Analog pH Sensor)
```cpp
const uint8_t PIN_PH = 34;  // ADC1_CH6

float readpH() {
  int raw = analogRead(PIN_PH);
  float voltage = raw * (3.3 / 4095.0);
  float ph = 7.0 + ((2.5 - voltage) / 0.18);  // สูตรคร่าวๆ
  
  return ph;
}
```

### 2. เพิ่มฟีเจอร์ Smart Features

#### ระบบประหยัดพลังงาน (Energy Saving Mode)
```cpp
bool energySavingMode = false;
int peakHourStart = 9;   // 09:00
int peakHourEnd = 22;    // 22:00

void checkEnergySaving() {
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  int hour = timeinfo.tm_hour;
  
  // เวลาพีค → ลดกำลังพัดลม 50%
  if (energySavingMode && hour >= peakHourStart && hour < peakHourEnd) {
    // ใช้ PWM ควบคุมความเร็วพัดลม
    ledcWrite(0, 128);  // 50% duty cycle
  }
}
```

#### ระบบแจ้งเตือนผ่าน LINE Notify
```cpp
#include <TridentTD_LineNotify.h>

#define LINE_TOKEN "Your_Line_Token_Here"

void sendLineNotify(String message) {
  LINE.notify(message);
}

// ตัวอย่างการใช้งาน
void autoWaterLevelControl() {
  if (iso1.isActive()) {
    relayPump.off();
    sendLineNotify("⚠️ แจ้งเตือน: ถังน้ำแห้ง! ปิดปั๊มอัตโนมัติ");
  }
}
```

#### ระบบบันทึกข้อมูล (Data Logging)
```cpp
#include <SD.h>

void logDataToSD() {
  File file = SD.open("/log.csv", FILE_APPEND);
  
  if (file) {
    // บันทึกในรูปแบบ CSV
    file.printf("%lu,%0.1f,%.1f,%.1f,%d,%d,%d\n",
                millis(),
                currentTemperature,
                xymd03_temperature,
                xymd03_humidity,
                relayFan.getState(),
                relayPump.getState(),
                relayHeater.getState());
    file.close();
  }
}

// เรียกใน loop() ทุก 1 นาที
if (millis() - lastLogTime >= 60000) {
  logDataToSD();
  lastLogTime = millis();
}
```

### 3. ปรับปรุง Web Dashboard

#### เพิ่ม Chart แสดงกราฟ (Chart.js)
```html
<!-- ใน index.html -->
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<canvas id="tempChart"></canvas>

<script>
const ctx = document.getElementById('tempChart').getContext('2d');
const chart = new Chart(ctx, {
  type: 'line',
  data: {
    labels: [],  // เวลา
    datasets: [{
      label: 'Temperature (°C)',
      data: [],  // ค่าอุณหภูมิ
      borderColor: 'rgb(255, 99, 132)',
    }]
  }
});

// อัปเดตข้อมูล
setInterval(() => {
  fetch('/api/sensors')
    .then(res => res.json())
    .then(data => {
      chart.data.labels.push(new Date().toLocaleTimeString());
      chart.data.datasets[0].data.push(data.ds18b20.temperature);
      chart.update();
    });
}, 5000);
</script>
```

#### เพิ่มระบบ Login (Authentication)
```cpp
// ใน setupWebServer()
server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
  if (!req->authenticate("admin", "password")) {
    return req->requestAuthentication();
  }
  req->send(SPIFFS, "/index.html", "text/html");
});
```

#### เพิ่ม Dark Mode
```html
<style>
body.dark-mode {
  background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
  color: #ffffff;
}

.card.dark-mode {
  background: rgba(30, 30, 46, 0.95);
}
</style>

<script>
function toggleDarkMode() {
  document.body.classList.toggle('dark-mode');
  localStorage.setItem('darkMode', document.body.classList.contains('dark-mode'));
}

// โหลดการตั้งค่า
if (localStorage.getItem('darkMode') === 'true') {
  document.body.classList.add('dark-mode');
}
</script>
```

### 4. เชื่อมต่อ Cloud Platforms

#### ส่งข้อมูลไป ThingSpeak
```cpp
#include <WiFi.h>
#include <HTTPClient.h>

String THINGSPEAK_API_KEY = "YOUR_API_KEY";

void sendToThingSpeak() {
  String url = "https://api.thingspeak.com/update?api_key=";
  url += THINGSPEAK_API_KEY;
  url += "&field1=" + String(currentTemperature);
  url += "&field2=" + String(xymd03_humidity);
  
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  http.end();
}

// เรียกใน loop() ทุก 15 วิ (ThingSpeak free tier)
```

#### เชื่อมต่อ Blynk IoT
```cpp
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"
#include <BlynkSimpleEsp32.h>

// ส่งข้อมูลไป Blynk
BLYNK_WRITE(V1) {  // Virtual Pin V1 = Fan Control
  int value = param.asInt();
  value ? relayFan.on() : relayFan.off();
}

void sendToBlynk() {
  Blynk.virtualWrite(V0, currentTemperature);
  Blynk.virtualWrite(V2, xymd03_humidity);
}
```

### 5. ระบบ Machine Learning

#### ทำนายอุณหภูมิด้วย TensorFlow Lite
```cpp
#include <TensorFlowLite_ESP32.h>

// โหลดโมเดล .tflite
const unsigned char model[] = {...};

void predictTemperature() {
  // เตรียมข้อมูล Input
  float input[24];  // อุณหภูมิย้อนหลัง 24 ชั่วโมง
  
  // Run Inference
  TfLiteTensor* input_tensor = interpreter->input(0);
  memcpy(input_tensor->data.f, input, sizeof(input));
  interpreter->Invoke();
  
  // ดึงผลลัพธ์
  TfLiteTensor* output = interpreter->output(0);
  float predicted_temp = output->data.f[0];
  
  Serial.printf("Predicted Temp in 1 hour: %.1f°C\n", predicted_temp);
}
```

### 6. การพัฒนาระบบ Hydroponics (ปลูกพืชไร้ดิน)

#### ควบคุม EC (Electrical Conductivity)
```cpp
float readEC() {
  // อ่านจาก EC Sensor (Analog)
  int raw = analogRead(PIN_EC);
  float ec = raw * 0.001;  // แปลงเป็น mS/cm
  return ec;
}

void autoECControl() {
  float ec = readEC();
  
  // ถ้า EC ต่ำ → เติมปุ๋ย
  if (ec < EC_MIN) {
    digitalWrite(PIN_FERTILIZER_PUMP, HIGH);
    delay(5000);  // เติม 5 วิ
    digitalWrite(PIN_FERTILIZER_PUMP, LOW);
  }
}
```

#### ควบคุม pH อัตโนมัติ
```cpp
void autopHControl() {
  float ph = readpH();
  
  // pH สูงเกิน → เติมกรด
  if (ph > PH_MAX) {
    digitalWrite(PIN_ACID_PUMP, HIGH);
    delay(2000);
    digitalWrite(PIN_ACID_PUMP, LOW);
  }
  // pH ต่ำเกิน → เติมเบส
  else if (ph < PH_MIN) {
    digitalWrite(PIN_BASE_PUMP, HIGH);
    delay(2000);
    digitalWrite(PIN_BASE_PUMP, LOW);
  }
}
```

### 7. ระบบควบคุมแบบ PID

#### ควบคุมอุณหภูมิด้วย PID Controller
```cpp
// PID Parameters
float Kp = 2.0, Ki = 0.5, Kd = 1.0;
float setpoint = 25.0;  // อุณหภูมิเป้าหมาย
float lastError = 0, integral = 0;

void pidTemperatureControl() {
  float temp = currentTemperature;
  float error = setpoint - temp;
  
  // P
  float P = Kp * error;
  
  // I
  integral += error;
  float I = Ki * integral;
  
  // D
  float derivative = error - lastError;
  float D = Kd * derivative;
  
  // PID Output
  float output = P + I + D;
  
  // ควบคุม PWM พัดลม/ฮีตเตอร์
  if (output > 0) {  // ต้องระบายความร้อน
    int fanSpeed = constrain(output * 10, 0, 255);
    ledcWrite(0, fanSpeed);
  } else {  // ต้องเพิ่มความร้อน
    int heaterPower = constrain(abs(output) * 10, 0, 255);
    ledcWrite(1, heaterPower);
  }
  
  lastError = error;
}
```

---

## 🔍 แนวทางแก้ไขปัญหา

### ปัญหาที่พบบ่อย

#### 1. ⚠️ ESP32 ไม่เชื่อมต่อ WiFi

**สาเหตุ**:
- SSID/Password ผิด
- WiFi 5GHz (ESP32 รองรับแค่ 2.4GHz)
- สัญญาณอ่อน

**วิธีแก้**:
```cpp
// ตรวจสอบ Serial Monitor
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}
Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

// ถ้ายัง connect ไม่ได้ → รีเซ็ต WiFi Config
// กด SW1 ค้าง 5 วิ
```

#### 2. ⚠️ เซ็นเซอร์ DS18B20 อ่านค่าไม่ได้ (-127°C)

**สาเหตุ**:
- ต่อสายผิด
- ไม่มี Pull-up Resistor (4.7kΩ)
- เซ็นเซอร์เสีย

**วิธีแก้**:
```cpp
// ตรวจสอบว่ามีเซ็นเซอร์
Serial.printf("Found %d devices\n", tempSensor.getDeviceCount());

// ใช้ค่าจำลองชั่วคราว
if (!sensorConnected) {
  simulateTemperature();  // ใช้ค่าจำลอง 25°C
}
```

#### 3. ⚠️ XY-MD03 อ่านค่าไม่ได้ (Timeout)

**สาเหตุ**:
- สาย RS485 ต่อกลับ (A ↔ B)
- Baud Rate ผิด
- Slave ID ผิด
- สาย TX/RX สลับกัน

**วิธีแก้**:
```cpp
// ลอง Baud Rate อื่น
xymd03.begin(4800);  // ลอง 4800
xymd03.begin(19200); // ลอง 19200

// ลอง Slave ID อื่น
DevTempHumidity xymd03(&Serial, 2);  // SlaveID=2

// ตรวจสอบสาย A/B
// ลองสลับสาย A ↔ B
```

#### 4. ⚠️ Relay ทำงานกลับกัน (Active Low Logic)

**สาเหตุ**:
- โมดูล Relay เป็น Active Low แต่โค้ดเป็น Active High

**วิธีแก้**:
```cpp
// แก้ใน DevRelay Constructor
DevRelayWithTimer relayFan(PIN_RELAY1, true);  // true = Active Low
//                                      ^^^^ ต้องเป็น true
```

#### 5. ⚠️ Web Dashboard แสดง "Failed to refresh data"

**สาเหตุ**:
- ESP32 ไม่ได้เชื่อม WiFi
- SPIFFS ยังไม่ Upload
- API Endpoint ผิด

**วิธีแก้**:
```bash
# 1. Upload SPIFFS ใหม่
pio run --target uploadfs

# 2. ตรวจสอบ IP Address
# เปิด Serial Monitor → ดู IP

# 3. ตรวจสอบ CORS
# เปิด Browser F12 → Console → ดู Error

# 4. ลอง Refresh
Ctrl+F5
```

#### 6. ⚠️ Schedule ไม่ทำงาน

**สาเหตุ**:
- NTP ไม่ได้ Sync
- เวลาผิด (Timezone)
- Schedule ไม่ได้ Enable

**วิธีแก้**:
```cpp
// ตรวจสอบเวลา
struct tm timeinfo;
if (getLocalTime(&timeinfo)) {
  Serial.printf("Current Time: %02d:%02d\n", 
                timeinfo.tm_hour, timeinfo.tm_min);
} else {
  Serial.println("Failed to get time!");
}

// แก้ Timezone (GMT+7 สำหรับไทย)
configTime(7*3600, 0, "pool.ntp.org");

// ตรวจสอบ Schedule Enabled
Serial.printf("Schedule Enabled: %s\n", scheduleEnabled ? "YES" : "NO");
```

#### 7. ⚠️ OLED Display ไม่แสดงผล

**สาเหตุ**:
- I2C Address ผิด (ลอง 0x3C หรือ 0x3D)
- สาย SDA/SCL สลับกัน
- แรงดัน 5V (ควรเป็น 3.3V)

**วิธีแก้**:
```cpp
// Scan I2C Address
#include <Wire.h>

void scanI2C() {
  for (byte i = 0x00; i < 0x7F; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Found I2C device at 0x%02X\n", i);
    }
  }
}

// ลอง Address อื่น
display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // ลอง 0x3D
```

### เครื่องมือ Debug

#### Serial Monitor
```cpp
// เปิดดูข้อความ Debug
Serial.printf("Temp: %.1f°C, Hum: %.1f%%\n", temp, hum);
```

#### LED Status
```cpp
// ใช้ Built-in LED แสดงสถานะ
digitalWrite(LED_BUILTIN, HIGH);  // ติด = กำลังทำงาน
delay(1000);
digitalWrite(LED_BUILTIN, LOW);   // ดับ = เสร็จแล้ว
```

---

## 📊 สรุปข้อมูลทางเทคนิค

### ตารางเปรียบเทียบ Sensors

| Sensor | ราคา | ความแม่นยำ | โปรโตคอล | การใช้งาน |
|--------|------|-----------|---------|----------|
| **DS18B20** | ~50 บาท | ±0.5°C | 1-Wire | อุณหภูมิอากาศ/น้ำ/ดิน |
| **XY-MD03** | ~300 บาท | ±0.5°C, ±3% RH | Modbus RTU | อุณหภูมิ+ความชื้นอากาศ |
| **DHT22** | ~120 บาท | ±0.5°C, ±2% RH | Digital | ทางเลือกแทน XY-MD03 |

### การใช้งาน GPIO

| GPIO | ฟังก์ชัน | ชนิด | หมายเหตุ |
|------|---------|------|----------|
| GPIO1 | UART TX | Output | XY-MD03 Modbus |
| GPIO3 | UART RX | Input | XY-MD03 Modbus |
| GPIO4 | Relay 1 (Fan) | Output | Active Low |
| GPIO14 | DS18B20 Data | I/O | 1-Wire, Pull-up 4.7kΩ |
| GPIO16 | Relay 2 (Pump) | Output | Active Low |
| GPIO17 | Relay 3 (Heater) | Output | Active Low |
| GPIO21 | I2C SDA | I/O | OLED Display |
| GPIO22 | I2C SCL | I/O | OLED Display |
| GPIO27 | ISO Input 2 | Input | Water Overflow |
| GPIO32 | Switch 3 (Up) | Input | Pull-up 10kΩ |
| GPIO33 | ISO Input 1 | Input | Tank Dry |
| GPIO34 | Switch 1 (Enter) | Input | Pull-up 10kΩ |
| GPIO35 | Switch 2 (Down) | Input | Pull-up 10kΩ |

### การบริโภคไฟฟ้า

| อุปกรณ์ | กระแส | กำลังไฟ |
|---------|-------|---------|
| ESP32 (Active) | ~160mA | ~0.5W |
| ESP32 (Deep Sleep) | ~10μA | ~0.00003W |
| OLED Display | ~20mA | ~0.07W |
| DS18B20 | ~1mA | ~0.003W |
| XY-MD03 | ~5mA | ~0.025W |
| Relay Module (1 ON) | ~70mA | ~0.35W |
| **รวม (ทุกอย่างเปิด)** | ~**250mA** | ~**0.8W** |

---

## 📚 แหล่งอ้างอิงและทรัพยากร

### เอกสารอ้างอิง
- [ESP32 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [ArduinoJson v7 Documentation](https://arduinojson.org/v7/)
- [ESPAsyncWebServer Library](https://github.com/me-no-dev/ESPAsyncWebServer)
- [OpenWeather API Docs](https://openweathermap.org/api)

### Libraries GitHub
- [WiFiManager](https://github.com/tzapu/WiFiManager)
- [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library)
- [ModbusMaster](https://github.com/4-20ma/ModbusMaster)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)

### ตัวอย่างโปรเจกต์ที่เกี่ยวข้อง
- [ESP32 Smart Greenhouse](https://github.com/examples/smart-greenhouse)
- [IoT Hydroponics System](https://github.com/examples/hydroponics)
- [ESP32 Weather Station](https://github.com/examples/weather-station)

---

## ✅ Checklist สำหรับผู้เริ่มต้น

### ก่อนเริ่มโปรเจกต์
- [ ] ติดตั้ง VS Code + PlatformIO แล้ว
- [ ] มีบอร์ด ESP32 DevKit V2
- [ ] เตรียมเซ็นเซอร์และอุปกรณ์ครบ
- [ ] มี WiFi สัญญาณดี (2.4GHz)
- [ ] มีความรู้พื้นฐาน C/C++

### ระหว่างพัฒนา
- [ ] ทดสอบเซ็นเซอร์ทีละตัว
- [ ] ตรวจสอบการต่อสายทุกครั้ง
- [ ] ใช้ Serial Monitor เช็ค Debug
- [ ] Upload SPIFFS ก่อน Upload Firmware
- [ ] บันทึกค่า API Key ไว้

### หลังเสร็จแล้ว
- [ ] ทดสอบ Manual Control ทุก Relay
- [ ] ทดสอบ Automation ทุกโหมด
- [ ] ทดสอบ Web Dashboard
- [ ] ทดสอบ WiFi Reset
- [ ] สำรองโค้ดไว้ (Git)

---

## 🎓 สรุป

ระบบ **ESP32 Smart Farm Control** นี้เป็นโปรเจกต์ที่ครบวงจรสำหรับการควบคุมฟาร์มอัจฉริยะ ครอบคลุมตั้งแต่:
- 🌡️ **ระบบตรวจวัด** - อุณหภูมิ, ความชื้น, คุณภาพอากาศ
- ⚙️ **ระบบควบคุม** - อัตโนมัติ 3 โหมด (Temp/Hum/Schedule)
- 🌐 **ระบบแสดงผล** - OLED + Web Dashboard
- 🔒 **ระบบป้องกัน** - Water Level Safety

โครงสร้างโค้ดออกแบบมาให้ **ขยายได้ง่าย** (Modular Design) สามารถเพิ่ม Sensors, Relays, หรือฟีเจอร์ใหม่ๆ ได้โดยไม่ต้องแก้โค้ดเดิมมาก

**จุดเด่น**:
- ✅ ใช้งานง่าย - ตั้งค่าผ่าน Web
- ✅ ประหยัดพลังงาน - Hysteresis Control
- ✅ ปลอดภัย - Water Level Protection
- ✅ รองรับอนาคต - Modular Architecture

**เหมาะสำหรับ**:
- นักศึกษาที่ทำโปรเจกต์ IoT
- Maker ที่สนใจ Smart Farming
- ผู้ที่ต้องการควบคุมฟาร์มระยะไกล
- ผู้ที่ต้องการต่อยอดเป็น Hydroponics

---

## 📧 ติดต่อและสนับสนุน

- **GitHub**: [thaitechzone/ESP32_DEV_SMARTFARM_CONTROL](https://github.com/thaitechzone/ESP32_DEV_SMARTFARM_CONTROL)
- **Email**: support@thaitechzone.com
- **Facebook**: ThaiTechZone Community
- **Line**: @thaitechzone

---

**เวอร์ชัน**: 1.0.0  
**อัปเดตล่าสุด**: March 4, 2026  
**ผู้พัฒนา**: ThaiTechZone Team  
**License**: MIT License

---

🌱 **Happy Smart Farming!** 🚜💚
