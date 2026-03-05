# N8N Integration Guide — ESP32 Smart Farm

## Overview Architecture

```
┌──────────┐  HTTP POST (JSON)  ┌──────────────┐  INSERT  ┌──────────────┐
│  ESP32   │ ─────────────────▶ │  N8N Webhook │ ────────▶│  PostgreSQL  │
│SmartFarm │                    │   Workflow   │          │  (ข้อมูล)    │
└──────────┘                    └──────┬───────┘          └──────┬───────┘
                                       │                          │
                              ┌────────▼────────┐       ┌────────▼──────┐
                              │  แจ้งเตือน LINE │       │    Grafana    │
                              │  Notify / Email │       │   Dashboard   │
                              └─────────────────┘       └───────────────┘
```

> ESP32 ส่งข้อมูลเซ็นเซอร์ทุก 1 นาที → N8N รับผ่าน Webhook → บันทึกลง PostgreSQL → แจ้งเตือนเมื่อค่าผิดปกติ

---

## Part 1: ESP32 — เพิ่ม Code ส่งข้อมูลไปยัง N8N

### 1.1 ตัวแปรที่ต้องเพิ่มใน `main.cpp`

```cpp
// ===== N8N INTEGRATION =====
const String N8N_WEBHOOK_URL = "http://YOUR_N8N_SERVER:5678/webhook/smartfarm";
unsigned long lastN8NPush = 0;
const unsigned long N8N_PUSH_INTERVAL = 60000; // ส่งทุก 60 วินาที
bool n8nEnabled = true;
```

### 1.2 Function ส่งข้อมูลไปยัง N8N

```cpp
// Push sensor data to N8N Webhook
void pushDataToN8N() {
  if (!wifiConnected || !n8nEnabled) return;

  HTTPClient http;
  http.begin(N8N_WEBHOOK_URL);
  http.addHeader("Content-Type", "application/json");

  // สร้าง JSON payload
  JsonDocument doc;

  // Timestamp (epoch seconds จาก NTP)
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[25];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S+07:00", &timeinfo);
    doc["timestamp"] = timeStr;
  }

  doc["device_id"]  = "smartfarm_esp32_01";
  doc["location"]   = "greenhouse_01";

  // DS18B20 Sensor
  JsonObject ds18b20 = doc["ds18b20"].to<JsonObject>();
  ds18b20["temperature"] = currentTemperature;
  ds18b20["connected"]   = sensorConnected;

  // XY-MD03 Sensor
  JsonObject xymd03 = doc["xymd03"].to<JsonObject>();
  xymd03["temperature"] = xymd03_temperature;
  xymd03["humidity"]    = xymd03_humidity;
  xymd03["connected"]   = xymd03_connected;

  // Tank Level Sensors
  JsonObject tanks = doc["tanks"].to<JsonObject>();
  tanks["tank1_dry"]      = iso1.isActive();   // true = น้ำแห้ง
  tanks["tank2_overflow"] = iso2.isActive();   // true = น้ำล้น

  // Relay States
  JsonObject relays = doc["relays"].to<JsonObject>();
  relays["fan"]    = relayFan.getState();
  relays["pump"]   = relayPump.getState();
  relays["heater"] = relayHeater.getState();

  // Automation Status
  JsonObject automation = doc["automation"].to<JsonObject>();
  automation["temp_auto_enabled"] = autoTempEnabled;
  automation["hum_auto_enabled"]  = autoHumEnabled;
  automation["schedule_enabled"]  = scheduleEnabled;

  // Weather (local outdoor)
  if (weatherDataAvailable) {
    JsonObject weather = doc["weather"].to<JsonObject>();
    weather["temperature"] = weather_temp;
    weather["humidity"]    = weather_humidity;
    weather["description"] = weather_description;
    weather["aqi"]         = aqi;
    weather["pm25"]        = pm2_5;
    weather["wind_speed"]  = weather_wind_speed;
  }

  String payload;
  serializeJson(doc, payload);

  Serial.println("N8N: Pushing data...");
  int httpCode = http.POST(payload);

  if (httpCode == 200) {
    Serial.println("N8N: Push OK");
  } else {
    Serial.printf("N8N: Push failed, code=%d\n", httpCode);
  }

  http.end();
}
```

### 1.3 เพิ่มใน `loop()` ของ `main.cpp`

```cpp
// Push data to N8N every 60 seconds
if (wifiConnected && (millis() - lastN8NPush >= N8N_PUSH_INTERVAL)) {
  lastN8NPush = millis();
  pushDataToN8N();
}
```

### 1.4 JSON Payload ตัวอย่างที่ ESP32 ส่งไป

```json
{
  "timestamp": "2026-03-04T14:30:00+07:00",
  "device_id": "smartfarm_esp32_01",
  "location": "greenhouse_01",
  "ds18b20": {
    "temperature": 28.5,
    "connected": true
  },
  "xymd03": {
    "temperature": 27.8,
    "humidity": 68.3,
    "connected": true
  },
  "tanks": {
    "tank1_dry": false,
    "tank2_overflow": false
  },
  "relays": {
    "fan": true,
    "pump": false,
    "heater": false
  },
  "automation": {
    "temp_auto_enabled": true,
    "hum_auto_enabled": false,
    "schedule_enabled": true
  },
  "weather": {
    "temperature": 32.1,
    "humidity": 75,
    "description": "few clouds",
    "aqi": 2,
    "pm25": 12.3,
    "wind_speed": 3.1
  }
}
```

---

## Part 2: PostgreSQL — Schema สำหรับเก็บข้อมูล

```sql
-- สร้าง database
CREATE DATABASE smartfarm;

-- ตาราง sensor readings (time-series)
CREATE TABLE sensor_readings (
    id          BIGSERIAL PRIMARY KEY,
    timestamp   TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    device_id   VARCHAR(50)  NOT NULL DEFAULT 'smartfarm_esp32_01',
    location    VARCHAR(100),

    -- DS18B20
    ds18b20_temp    FLOAT,
    ds18b20_ok      BOOLEAN,

    -- XY-MD03
    xymd03_temp     FLOAT,
    xymd03_hum      FLOAT,
    xymd03_ok       BOOLEAN,

    -- Tanks
    tank1_dry       BOOLEAN,
    tank2_overflow  BOOLEAN,

    -- Relays
    fan_on      BOOLEAN,
    pump_on     BOOLEAN,
    heater_on   BOOLEAN,

    -- Weather
    weather_temp    FLOAT,
    weather_hum     FLOAT,
    weather_aqi     INT,
    weather_pm25    FLOAT,
    weather_desc    VARCHAR(100)
);

-- Index สำหรับ query เร็ว
CREATE INDEX idx_sensor_time ON sensor_readings (timestamp DESC);
CREATE INDEX idx_sensor_device ON sensor_readings (device_id, timestamp DESC);

-- ตาราง alerts log
CREATE TABLE alert_log (
    id          BIGSERIAL PRIMARY KEY,
    timestamp   TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    device_id   VARCHAR(50),
    alert_type  VARCHAR(50),  -- 'HIGH_TEMP', 'LOW_HUM', 'TANK_DRY', etc.
    message     TEXT,
    value       FLOAT,
    threshold   FLOAT,
    notified    BOOLEAN DEFAULT false
);
```

---

## Part 3: N8N Workflows

### Workflow 1 — รับข้อมูล ESP32 และบันทึกลง PostgreSQL

```
[Webhook] → [Set Node] → [PostgreSQL Insert] → [IF: ค่าผิดปกติ?]
                                                      │
                                              [LINE Notify Alert]
```

**Node 1: Webhook**
- Method: `POST`
- Path: `/smartfarm`
- Response: Immediately

**Node 2: Set Node** — เตรียมข้อมูลสำหรับ INSERT
```javascript
// ใส่ใน Expression ของแต่ละ field
timestamp:   {{ $json.timestamp }}
device_id:   {{ $json.device_id }}
location:    {{ $json.location }}
ds18b20_temp:{{ $json.ds18b20.temperature }}
ds18b20_ok:  {{ $json.ds18b20.connected }}
xymd03_temp: {{ $json.xymd03.temperature }}
xymd03_hum:  {{ $json.xymd03.humidity }}
xymd03_ok:   {{ $json.xymd03.connected }}
tank1_dry:   {{ $json.tanks.tank1_dry }}
tank2_over:  {{ $json.tanks.tank2_overflow }}
fan_on:      {{ $json.relays.fan }}
pump_on:     {{ $json.relays.pump }}
heater_on:   {{ $json.relays.heater }}
weather_temp:{{ $json.weather?.temperature ?? null }}
weather_hum: {{ $json.weather?.humidity ?? null }}
weather_aqi: {{ $json.weather?.aqi ?? null }}
weather_pm25:{{ $json.weather?.pm25 ?? null }}
weather_desc:{{ $json.weather?.description ?? '' }}
```

**Node 3: PostgreSQL — Insert**
```sql
INSERT INTO sensor_readings (
    timestamp, device_id, location,
    ds18b20_temp, ds18b20_ok,
    xymd03_temp, xymd03_hum, xymd03_ok,
    tank1_dry, tank2_overflow,
    fan_on, pump_on, heater_on,
    weather_temp, weather_hum, weather_aqi, weather_pm25, weather_desc
) VALUES (
    $1, $2, $3, $4, $5, $6, $7, $8, $9,
    $10, $11, $12, $13, $14, $15, $16, $17, $18
)
```

**Node 4: IF — ตรวจสอบค่าผิดปกติ**
```javascript
// Conditions (ตั้งค่า OR)
{{ $json.ds18b20.temperature > 35 }}          // อุณหภูมิสูงเกิน
{{ $json.ds18b20.temperature < 15 }}          // อุณหภูมิต่ำเกิน
{{ $json.xymd03.humidity < 40 }}              // ความชื้นต่ำเกิน
{{ $json.xymd03.humidity > 90 }}              // ความชื้นสูงเกิน
{{ $json.tanks.tank1_dry === true }}          // น้ำแห้ง ‼️
{{ $json.tanks.tank2_overflow === true }}     // น้ำล้น ‼️
```

---

### Workflow 2 — แจ้งเตือน LINE Notify

```
[IF True] → [Function: สร้างข้อความ] → [HTTP Request: LINE Notify]
```

**Function Node — สร้างข้อความแจ้งเตือน**
```javascript
const d = $input.item.json;
const temp = d.ds18b20?.temperature?.toFixed(1) ?? '--';
const hum  = d.xymd03?.humidity?.toFixed(1) ?? '--';

let alerts = [];

if (d.ds18b20?.temperature > 35)
  alerts.push(`🔥 อุณหภูมิสูง: ${temp}°C (เกิน 35°C)`);
if (d.ds18b20?.temperature < 15)
  alerts.push(`🥶 อุณหภูมิต่ำ: ${temp}°C (ต่ำกว่า 15°C)`);
if (d.xymd03?.humidity < 40)
  alerts.push(`🏜️ ความชื้นต่ำ: ${hum}% (ต่ำกว่า 40%)`);
if (d.xymd03?.humidity > 90)
  alerts.push(`💦 ความชื้นสูง: ${hum}% (เกิน 90%)`);
if (d.tanks?.tank1_dry)
  alerts.push(`⚠️ น้ำในถังแห้ง! ปั๊มหยุดทำงาน`);
if (d.tanks?.tank2_overflow)
  alerts.push(`🚨 น้ำล้นถัง! กรุณาตรวจสอบ`);

const ts = new Date().toLocaleString('th-TH', { timeZone: 'Asia/Bangkok' });

const message = [
  `\n🌱 Smart Farm Alert`,
  `📍 ${d.location ?? 'greenhouse_01'}`,
  `🕐 ${ts}`,
  ``,
  ...alerts,
  ``,
  `📊 ค่าปัจจุบัน:`,
  `  DS18B20: ${temp}°C`,
  `  XY-MD03: ${temp}°C / ${hum}%`,
  `  พัดลม: ${d.relays?.fan ? 'ON':'OFF'} | ปั๊ม: ${d.relays?.pump ? 'ON':'OFF'} | ฮีตเตอร์: ${d.relays?.heater ? 'ON':'OFF'}`
].join('\n');

return [{ json: { message } }];
```

**HTTP Request Node — LINE Notify**
- Method: `POST`
- URL: `https://notify-api.line.me/api/notify`
- Headers:
  - `Authorization`: `Bearer YOUR_LINE_NOTIFY_TOKEN`
  - `Content-Type`: `application/x-www-form-urlencoded`
- Body: `message={{ $json.message }}`

---

### Workflow 3 — Daily Summary Report (ทุกเย็น 18:00)

```
[Cron: 0 18 * * *] → [PostgreSQL Query] → [Function: สรุปข้อมูล] → [LINE Notify]
```

**Cron Node**
- Expression: `0 18 * * *` (ทุกวัน 18:00 น.)

**PostgreSQL Query**
```sql
SELECT
    DATE_TRUNC('day', timestamp) AS date,
    COUNT(*)                      AS readings,
    ROUND(AVG(ds18b20_temp)::numeric, 1) AS avg_temp,
    ROUND(MAX(ds18b20_temp)::numeric, 1) AS max_temp,
    ROUND(MIN(ds18b20_temp)::numeric, 1) AS min_temp,
    ROUND(AVG(xymd03_hum)::numeric, 1)  AS avg_hum,
    SUM(CASE WHEN tank1_dry = true THEN 1 ELSE 0 END) AS tank_dry_events,
    SUM(CASE WHEN pump_on = true THEN 1 ELSE 0 END)   AS pump_on_count
FROM sensor_readings
WHERE timestamp >= NOW() - INTERVAL '24 hours'
GROUP BY DATE_TRUNC('day', timestamp);
```

---

### Workflow 4 — บันทึกลง Google Sheets (ตัวเลือกทดแทน)

```
[Webhook] → [Google Sheets: Append Row]
```

**Google Sheets Node (Append)**
| Column | Expression |
|--------|-----------|
| Timestamp | `{{ $json.timestamp }}` |
| DS18B20 Temp | `{{ $json.ds18b20.temperature }}` |
| XY-MD03 Temp | `{{ $json.xymd03.temperature }}` |
| Humidity | `{{ $json.xymd03.humidity }}` |
| Tank1 Dry | `{{ $json.tanks.tank1_dry }}` |
| Tank2 Overflow | `{{ $json.tanks.tank2_overflow }}` |
| Fan | `{{ $json.relays.fan }}` |
| Pump | `{{ $json.relays.pump }}` |
| Heater | `{{ $json.relays.heater }}` |
| AQI | `{{ $json.weather?.aqi }}` |

---

## Part 4: Grafana — เชื่อมต่อกับ PostgreSQL

หลังจาก N8N บันทึกข้อมูลลง PostgreSQL แล้ว Grafana สามารถดึง query ได้ทันที

### Dashboard Panels ที่แนะนำ

**Panel 1: Temperature Timeline**
```sql
SELECT
  timestamp AS "time",
  ds18b20_temp AS "DS18B20 (°C)",
  xymd03_temp  AS "XY-MD03 (°C)"
FROM sensor_readings
WHERE timestamp BETWEEN $__timeFrom() AND $__timeTo()
  AND device_id = 'smartfarm_esp32_01'
ORDER BY timestamp;
```

**Panel 2: Humidity Timeline**
```sql
SELECT
  timestamp AS "time",
  xymd03_hum AS "Humidity (%)"
FROM sensor_readings
WHERE timestamp BETWEEN $__timeFrom() AND $__timeTo()
ORDER BY timestamp;
```

**Panel 3: Relay ON/OFF Timeline**
```sql
SELECT
  timestamp AS "time",
  CASE WHEN fan_on    THEN 1 ELSE 0 END AS "Fan",
  CASE WHEN pump_on   THEN 1 ELSE 0 END AS "Pump",
  CASE WHEN heater_on THEN 1 ELSE 0 END AS "Heater"
FROM sensor_readings
WHERE timestamp BETWEEN $__timeFrom() AND $__timeTo()
ORDER BY timestamp;
```

**Panel 4: Tank Events**
```sql
SELECT
  timestamp AS "time",
  CASE WHEN tank1_dry      THEN 1 ELSE 0 END AS "Tank Dry ‼️",
  CASE WHEN tank2_overflow THEN 1 ELSE 0 END AS "Overflow ‼️"
FROM sensor_readings
WHERE timestamp BETWEEN $__timeFrom() AND $__timeTo()
ORDER BY timestamp;
```

---

## Part 5: N8N Setup Checklist

### ติดตั้ง N8N ด้วย Docker

```bash
# docker-compose.yml
version: '3.8'
services:
  n8n:
    image: n8nio/n8n
    ports:
      - "5678:5678"
    environment:
      - N8N_HOST=0.0.0.0
      - N8N_PORT=5678
      - N8N_PROTOCOL=http
      - WEBHOOK_URL=http://YOUR_SERVER_IP:5678/
      - DB_TYPE=postgresdb
      - DB_POSTGRESDB_HOST=postgres
      - DB_POSTGRESDB_PORT=5432
      - DB_POSTGRESDB_DATABASE=n8n
      - DB_POSTGRESDB_USER=n8n
      - DB_POSTGRESDB_PASSWORD=n8npassword
    volumes:
      - n8n_data:/home/node/.n8n
    depends_on:
      - postgres

  postgres:
    image: postgres:16
    environment:
      - POSTGRES_USER=n8n
      - POSTGRES_PASSWORD=n8npassword
      - POSTGRES_DB=n8n
    volumes:
      - postgres_data:/var/lib/postgresql/data

volumes:
  n8n_data:
  postgres_data:
```

```bash
docker compose up -d
# เปิด browser: http://YOUR_SERVER_IP:5678
```

---

### Checklist N8N

- [ ] ติดตั้ง N8N บน Raspberry Pi / VPS
- [ ] สร้าง Webhook node → คัดลอก URL
- [ ] อัพเดต `N8N_WEBHOOK_URL` ใน `main.cpp`
- [ ] สร้าง database `smartfarm` และ table `sensor_readings` ใน PostgreSQL
- [ ] เชื่อมต่อ PostgreSQL Credentials ใน N8N
- [ ] ทดสอบส่งข้อมูลจาก ESP32 → ดู Webhook execution log
- [ ] ตั้งค่า LINE Notify Token
- [ ] เปิด Workflow และ `Activate`
- [ ] เชื่อมต่อ Grafana กับ PostgreSQL → สร้าง Dashboard

---

### สรุป Webhook URL ที่ใช้

| Workflow | URL |
|---|---|
| รับข้อมูล + บันทึก | `http://YOUR_N8N:5678/webhook/smartfarm` |
| ทดสอบ (test mode) | `http://YOUR_N8N:5678/webhook-test/smartfarm` |

> **หมายเหตุ:** เปลี่ยน `YOUR_N8N` เป็น IP จริงของเครื่องที่รัน N8N  
> ESP32 กับ N8N ต้องอยู่ใน Network เดียวกัน หรือใช้ Public IP / Domain
