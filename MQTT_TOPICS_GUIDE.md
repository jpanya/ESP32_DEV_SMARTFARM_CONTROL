# MQTT Topics Guide - Smart Farm Control

## 📡 MQTT Configuration

### Default Settings
- **MQTT Broker:** broker.hivemq.com
- **MQTT Port:** 1883
- **Topic Prefix:** smartfarm/device01 (customizable)

### HiveMQ Public Broker
- No authentication required for public broker
- For HiveMQ Cloud, use username/password authentication

---

## 🎯 MQTT Topics Structure

All topics use the prefix format: `{PREFIX}/...`

Example with prefix `smartfarm/device01`:

### 📊 Published Topics (ESP32 → Server)

#### 1. Sensor Data
**Topic:** `{PREFIX}/sensors`

**Published:** Every 10 seconds

**Payload (JSON):**
```json
{
  "ds18b20": {
    "temperature": 25.5,
    "connected": true
  },
  "xymd03": {
    "temperature": 26.3,
    "humidity": 65.5,
    "connected": true
  },
  "weather": {
    "temperature": 28.0,
    "humidity": 70.0,
    "description": "Clear sky"
  },
  "iso1": false,
  "iso2": false
}
```

**Example Topic:** `smartfarm/device01/sensors`

---

#### 2. Relay Status
**Topic:** `{PREFIX}/relays/status`

**Published:** When relay state changes

**Payload (JSON):**
```json
{
  "relay1": true,
  "relay2": false,
  "relay3": true
}
```

**Example Topic:** `smartfarm/device01/relays/status`

---

### 🎮 Subscribed Topics (Server → ESP32)

#### 1. Control Fan (Relay 1)
**Topic:** `{PREFIX}/relay/1/set`

**Payload:** `ON` or `OFF` (or `1` or `0`)

**Example:**
```
Topic: smartfarm/device01/relay/1/set
Message: ON
```

---

#### 2. Control Pump (Relay 2)
**Topic:** `{PREFIX}/relay/2/set`

**Payload:** `ON` or `OFF` (or `1` or `0`)

**Example:**
```
Topic: smartfarm/device01/relay/2/set
Message: OFF
```

---

#### 3. Control Heater (Relay 3)
**Topic:** `{PREFIX}/relay/3/set`

**Payload:** `ON` or `OFF` (or `1` or `0`)

**Example:**
```
Topic: smartfarm/device01/relay/3/set
Message: ON
```

---

## 🔧 Configuration via Dashboard

### ตั้งค่าผ่าน Web Dashboard

1. เปิด Dashboard: `http://{ESP32_IP}/settings`
2. เลื่อนไปที่ **MQTT Settings (HiveMQ)**
3. ตั้งค่า:
   - ✅ เปิดใช้งาน MQTT
   - 🌐 MQTT Server: `broker.hivemq.com`
   - 🔌 MQTT Port: `1883`
   - 🔑 Topic Prefix: `smartfarm/device01` (เปลี่ยนได้ เช่น `smartfarm/greenhouse1`)
   - 👤 Username: (ถ้าใช้ HiveMQ Cloud)
   - 🔒 Password: (ถ้าใช้ HiveMQ Cloud)
4. กด **💾 บันทึกการตั้งค่า**
5. ดูสถานะ MQTT และ Topics ที่ dashboard

---

## 📱 ตัวอย่างการใช้งาน

### 1. ใช้ MQTT Explorer (Desktop App)

1. Download: [MQTT Explorer](http://mqtt-explorer.com/)
2. เชื่อมต่อกับ `broker.hivemq.com` port `1883`
3. Subscribe to: `smartfarm/#` (รับข้อมูลทั้งหมด)
4. Publish to relay topics เพื่อควบคุมอุปกรณ์

---

### 2. ใช้ MQTT Client Library (Python)

```python
import paho.mqtt.client as mqtt
import json

# Callback when connected
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    # Subscribe to sensor data
    client.subscribe("smartfarm/device01/sensors")
    client.subscribe("smartfarm/device01/relays/status")

# Callback when message received
def on_message(client, userdata, msg):
    print(f"Topic: {msg.topic}")
    data = json.loads(msg.payload.decode())
    print(f"Data: {json.dumps(data, indent=2)}")

# Create MQTT client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to HiveMQ
client.connect("broker.hivemq.com", 1883, 60)

# Control relay
def control_relay(relay_num, state):
    topic = f"smartfarm/device01/relay/{relay_num}/set"
    message = "ON" if state else "OFF"
    client.publish(topic, message)
    print(f"Published to {topic}: {message}")

# Example: Turn on fan
control_relay(1, True)

# Start loop
client.loop_forever()
```

---

### 3. ใช้ Node-RED

1. ติดตั้ง Node-RED: `npm install -g node-red`
2. รัน: `node-red`
3. เปิด browser: `http://localhost:1880`
4. เพิ่ม MQTT nodes:
   - MQTT In (subscribe)
   - MQTT Out (publish)
5. ตั้งค่า broker: `broker.hivemq.com:1883`
6. Subscribe to: `smartfarm/device01/#`
7. สร้าง flow สำหรับควบคุมและแสดงผล

---

### 4. ใช้ Home Assistant

```yaml
# configuration.yaml
mqtt:
  broker: broker.hivemq.com
  port: 1883

sensor:
  - platform: mqtt
    name: "Smart Farm Temperature"
    state_topic: "smartfarm/device01/sensors"
    value_template: "{{ value_json.ds18b20.temperature }}"
    unit_of_measurement: "°C"
  
  - platform: mqtt
    name: "Smart Farm Humidity"
    state_topic: "smartfarm/device01/sensors"
    value_template: "{{ value_json.xymd03.humidity }}"
    unit_of_measurement: "%"

switch:
  - platform: mqtt
    name: "Smart Farm Fan"
    command_topic: "smartfarm/device01/relay/1/set"
    state_topic: "smartfarm/device01/relays/status"
    value_template: "{{ 'ON' if value_json.relay1 else 'OFF' }}"
    payload_on: "ON"
    payload_off: "OFF"
```

---

## 🔒 Security (HiveMQ Cloud)

สำหรับการใช้งาน HiveMQ Cloud (มีความปลอดภัยสูง):

1. สมัคร HiveMQ Cloud Account: https://www.hivemq.com/mqtt-cloud-broker/
2. สร้าง Cluster
3. เพิ่ม Credentials (Username/Password)
4. ตั้งค่าใน Dashboard:
   - Server: `{your-cluster}.s1.eu.hivemq.cloud`
   - Port: `8883` (TLS) หรือ `1883` (Non-TLS)
   - Username: `{your-username}`
   - Password: `{your-password}`

---

## 🎯 ป้องกัน Topic ซ้ำซ้อน

เมื่อใช้งานหลายบอร์ดพร้อมกัน ให้ตั้งค่า **Topic Prefix** ที่ต่างกัน:

```
Device 1: smartfarm/greenhouse1
Device 2: smartfarm/greenhouse2
Device 3: smartfarm/nursery1
Device 4: smartfarm/outdoor1
```

---

## 📊 API Endpoints

### Get MQTT Configuration
```bash
GET /api/mqtt/config
```

### Update MQTT Configuration
```bash
POST /api/mqtt/config
Content-Type: application/json

{
  "enabled": true,
  "server": "broker.hivemq.com",
  "port": 1883,
  "username": "",
  "password": "",
  "topicPrefix": "smartfarm/device01"
}
```

### Get MQTT Topics and Status
```bash
GET /api/mqtt/topics
```

### Get MQTT Status Only
```bash
GET /api/mqtt/status
```

---

## 🐛 Troubleshooting

### MQTT ไม่เชื่อมต่อ

1. ตรวจสอบว่า WiFi เชื่อมต่อแล้ว
2. ตรวจสอบ MQTT Server และ Port
   - HiveMQ Public: `broker.hivemq.com:1883`
3. ตรวจสอบ Username/Password (ถ้าใช้ HiveMQ Cloud)
4. ลอง Restart ESP32
5. ดู Serial Monitor เพื่อดู error messages

### ไม่ได้รับข้อมูลจาก Topic

1. ตรวจสอบว่า MQTT เชื่อมต่อสำเร็จ (ดูใน Dashboard)
2. ตรวจสอบ Topic Prefix ให้ถูกต้อง
3. ใช้ MQTT Explorer subscribe to `#` เพื่อดู topics ทั้งหมด

### Relay ไม่ทำงาน

1. ตรวจสอบ Payload: ต้องเป็น `ON` หรือ `OFF` (หรือ `1` หรือ `0`)
2. ตรวจสอบ Topic: `{PREFIX}/relay/{1,2,3}/set`
3. ดู Serial Monitor เพื่อดู MQTT messages

---

## 📚 Additional Resources

- [HiveMQ Public Broker](https://www.hivemq.com/public-mqtt-broker/)
- [HiveMQ Cloud](https://www.hivemq.com/mqtt-cloud-broker/)
- [MQTT Explorer](http://mqtt-explorer.com/)
- [Paho MQTT Python](https://github.com/eclipse/paho.mqtt.python)
- [Node-RED](https://nodered.org/)
- [Home Assistant MQTT](https://www.home-assistant.io/integrations/mqtt/)

---

**Created by:** Smart Farm Control System  
**Version:** 1.0  
**Date:** March 2026
