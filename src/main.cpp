#include <Arduino.h>
#include "DevIsoInput.h"
#include "DevRelay.h"
#include "DevSwitch.h"
#include "DevTempHumidity.h"

// OLED display library
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi Manager
#include <WiFi.h>
#include <WiFiManager.h>

// DS18B20 Temperature Sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// OLED display configuration
#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 128
#endif
#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 64
#endif
#ifndef OLED_RESET
#define OLED_RESET -1
#endif
#ifndef SCREEN_ADDRESS
#define SCREEN_ADDRESS 0x3C
#endif

// Instantiate OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// DS18B20 Temperature Sensor Configuration
const uint8_t PIN_TEMP_SENSOR = 14; // GPIO14 for DS18B20 Data Pin
OneWire oneWire(PIN_TEMP_SENSOR);
DallasTemperature tempSensor(&oneWire);

// Temperature variables
float currentTemperature = 0.0;
bool sensorConnected = false;
unsigned long lastTempUpdate = 0;
const unsigned long TEMP_UPDATE_INTERVAL = 2000; // Update every 2 seconds
float simulatedTemp = 25.0; // Starting simulated temperature
float tempDelta = 0.5; // Simulated temperature change rate

// XY-MD03 Temperature & Humidity Sensor (Modbus RTU via Serial0/RS485)
DevTempHumidity xymd03(&Serial, 1); // Serial0 (GPIO1=TX, GPIO3=RX), Slave ID=1
float xymd03_temperature = 0.0;
float xymd03_humidity = 0.0;
bool xymd03_connected = false;
unsigned long lastXYMD03Update = 0;
const unsigned long XYMD03_UPDATE_INTERVAL = 3000; // Update every 3 seconds
// Simulation variables for XY-MD03
float simTempXY = 26.5;
float simHumXY = 65.0;
float simTempDeltaXY = 0.3;
float simHumDeltaXY = 1.0;

// Display page management
uint8_t currentPage = 0; // 0=Main page, 1=XY-MD03 page
const uint8_t MAX_PAGES = 2;

// Pin definitions (from HardwareESP32Config.md)
const uint8_t PIN_SW1 = 34; // SW1 = Enter/Select (Active Low)
const uint8_t PIN_SW2 = 35; // SW2 = Down (Active Low)
const uint8_t PIN_SW3 = 32; // SW3 = Up (Active Low)

// Instantiate switches (Active Low)
DevSwitch sw1(PIN_SW1, false);
DevSwitch sw2(PIN_SW2, false);
DevSwitch sw3(PIN_SW3, false);

// Relay pin definitions (from HardwareESP32Config.md)
const uint8_t PIN_RELAY1 = 4;  // Relay1 = Fan (Active Low)
const uint8_t PIN_RELAY2 = 16; // Relay2 = Pump (Active Low)
const uint8_t PIN_RELAY3 = 17; // Relay3 = Heater (Active Low)

// Instantiate relays (Active Low)
DevRelayWithTimer relayFan(PIN_RELAY1, true);
DevRelayWithTimer relayPump(PIN_RELAY2, true);
DevRelayWithTimer relayHeater(PIN_RELAY3, true);

// Isolated inputs (from HardwareESP32Config.md)
const uint8_t PIN_ISO1 = 33; // ISO1 = TankLevelSensor1 (water dry) Active Low
const uint8_t PIN_ISO2 = 27; // ISO2 = TankLevelSensor2 (water overflow) Active Low

// Instantiate isolated inputs (Active Low)
DevIsoInput iso1(PIN_ISO1, false);
DevIsoInput iso2(PIN_ISO2, false);

// Display update timing
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_INTERVAL = 250; // ms

// WiFi variables
String ipAddress = "Not Connected";
bool wifiConnected = false;

// WiFi Reset detection
const unsigned long WIFI_RESET_HOLD_TIME = 5000; // 5 seconds
unsigned long sw1PressStart = 0;
bool sw1LongPressHandled = false;

// Forward declarations
void showWelcome();
void updateDisplay();
void updateDisplayPage0(); // Main page
void updateDisplayPage1(); // XY-MD03 page
void showCountdown(int seconds);
void setupWiFi();
void checkWiFiResetButton();
void readTemperature();
void simulateTemperature();
void readXYMD03();
void simulateXYMD03();
void switchPage();

// Callback handlers
void onSw1Click() {
  // SW1: Switch page
  switchPage();
}

void onSw2Click() {
  // SW2: Down (reserved for future use)
}

void onSw3Click() {
  // SW3: Up (reserved for future use)
}

// Relay control helpers
void toggleFan() {
  relayFan.toggle();
  Serial.printf("Fan: %s\n", relayFan.getState() ? "ON" : "OFF");
}

void togglePump() {
  relayPump.toggle();
  Serial.printf("Pump: %s\n", relayPump.getState() ? "ON" : "OFF");
}

void toggleHeater() {
  relayHeater.toggle();
  Serial.printf("Heater: %s\n", relayHeater.getState() ? "ON" : "OFF");
}

// ISO callbacks
void onIso1Active() {
  Serial.println("ISO1: TankLevelSensor1 - DRY (active)");
}

void onIso1Inactive() {
  Serial.println("ISO1: TankLevelSensor1 - OK (inactive)");
}

void onIso2Active() {
  Serial.println("ISO2: TankLevelSensor2 - OVERFLOW (active)");
}

void onIso2Inactive() {
  Serial.println("ISO2: TankLevelSensor2 - OK (inactive)");
}

// --- OLED helpers ---
void showWelcome() {
  display.clearDisplay();
  
  // Draw frame
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.drawRect(1, 1, SCREEN_WIDTH-2, SCREEN_HEIGHT-2, SSD1306_WHITE);
  
  // Title
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(8, 8);
  display.print("ESP32");
  display.setCursor(8, 24);
  display.print("DevKit V2");
  
  // Subtitle
  display.setTextSize(1);
  display.setCursor(10, 43);
  display.print("Smart Farm Control");
  
  // WiFi Status
  display.setTextSize(1);
  if (wifiConnected) {
    // Show IP Address (center align bottom)
    display.setCursor(6, 56);
    display.print("IP:");
    display.print(ipAddress);
  } else {
    // Show SSID for config
    display.setCursor(12, 56);
    display.print("SSID:ESP32_Farm");
  }
  
  display.display();
  delay(3000);
}

void showCountdown(int seconds) {
  display.clearDisplay();
  
  // Border
  display.drawRect(10, 10, SCREEN_WIDTH-20, SCREEN_HEIGHT-20, SSD1306_WHITE);
  
  // Warning message
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(16, 16);
  display.println("WiFi Reset in:");
  
  // Countdown number (large)
  display.setTextSize(3);
  display.setCursor(52, 32);
  display.println(seconds);
  
  display.display();
}

void setupWiFi() {
  display.clearDisplay();
  
  // Draw border
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  
  // Title
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(16, 8);
  display.print("WiFi Setup Mode");
  display.drawFastHLine(4, 18, SCREEN_WIDTH-8, SSD1306_WHITE);
  
  // Instructions
  display.setCursor(6, 24);
  display.print("Connect to WiFi:");
  
  // SSID (highlighted)
  display.setTextSize(2);
  display.setCursor(6, 36);
  display.print("ESP32_Farm");
  
  // Bottom instruction
  display.setTextSize(1);
  display.setCursor(4, 54);
  display.print("IP:192.168.4.1");
  
  display.display();
  
  Serial.println("Starting WiFi Manager...");
  Serial.println("SSID: ESP32_Farm");
  Serial.println("IP: 192.168.4.1");
  
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(180); // 3 minutes timeout
  
  // Try to connect
  if (wifiManager.autoConnect("ESP32_Farm")) {
    wifiConnected = true;
    ipAddress = WiFi.localIP().toString();
    Serial.println("WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(ipAddress);
    
    // Show success message
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(20, 24);
    display.print("WiFi Connected!");
    display.setCursor(6, 36);
    display.print("IP:");
    display.print(ipAddress);
    display.display();
    delay(2000);
  } else {
    wifiConnected = false;
    ipAddress = "Not Connected";
    Serial.println("WiFi connection failed");
  }
}

// Check for WiFi reset button during startup
void checkWiFiResetButton() {
  // Initialize SW1 for reset check
  pinMode(PIN_SW1, INPUT_PULLUP);
  
  // Check if SW1 is pressed at startup
  bool sw1Pressed = (digitalRead(PIN_SW1) == LOW); // Active Low
  
  if (sw1Pressed) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(8, 20);
    display.print("Hold for WiFi Reset");
    display.display();
    
    unsigned long pressStart = millis();
    bool resetTriggered = false;
    
    // Wait and check if button held for 5 seconds
    while ((millis() - pressStart) < WIFI_RESET_HOLD_TIME) {
      // Check if button is still pressed
      if (digitalRead(PIN_SW1) != LOW) {
        // Button released
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(20, 28);
        display.print("Reset Cancelled");
        display.display();
        delay(1000);
        return;
      }
      
      // Show countdown
      unsigned long elapsed = millis() - pressStart;
      if (elapsed >= 1000) {
        int remainingSeconds = 5 - (elapsed / 1000);
        if (remainingSeconds >= 0) {
          showCountdown(remainingSeconds);
        }
      }
      
      delay(100);
    }
    
    // Button held for 5 seconds - reset WiFi
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(8, 24);
    display.print("Resetting WiFi...");
    display.display();
    
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    delay(1000);
    
    display.clearDisplay();
    display.setCursor(8, 24);
    display.print("WiFi Reset!");
    display.setCursor(8, 36);
    display.print("Rebooting...");
    display.display();
    delay(2000);
    
    ESP.restart();
  }
}

// Simulate temperature when sensor is not connected
void simulateTemperature() {
  // Simulate temperature fluctuation between 20°C and 30°C
  simulatedTemp += tempDelta;
  
  if (simulatedTemp >= 30.0) {
    tempDelta = -0.5;
    simulatedTemp = 30.0;
  } else if (simulatedTemp <= 20.0) {
    tempDelta = 0.5;
    simulatedTemp = 20.0;
  }
  
  currentTemperature = simulatedTemp;
}

// Read temperature from DS18B20 sensor
void readTemperature() {
  tempSensor.requestTemperatures();
  float tempC = tempSensor.getTempCByIndex(0);
  
  // Check if reading is valid (DS18B20 returns -127 or 85 if sensor is disconnected)
  if (tempC != DEVICE_DISCONNECTED_C && tempC != 85.0 && tempC > -50.0 && tempC < 125.0) {
    sensorConnected = true;
    currentTemperature = tempC;
  } else {
    sensorConnected = false;
    simulateTemperature();
  }
}

// Simulate XY-MD03 values when sensor is not connected
void simulateXYMD03() {
  // Simulate temperature fluctuation between 23°C and 30°C
  simTempXY += simTempDeltaXY;
  if (simTempXY >= 30.0) {
    simTempDeltaXY = -0.3;
    simTempXY = 30.0;
  } else if (simTempXY <= 23.0) {
    simTempDeltaXY = 0.3;
    simTempXY = 23.0;
  }
  
  // Simulate humidity fluctuation between 50% and 80%
  simHumXY += simHumDeltaXY;
  if (simHumXY >= 80.0) {
    simHumDeltaXY = -1.0;
    simHumXY = 80.0;
  } else if (simHumXY <= 50.0) {
    simHumDeltaXY = 1.0;
    simHumXY = 50.0;
  }
  
  xymd03_temperature = simTempXY;
  xymd03_humidity = simHumXY;
}

// Read XY-MD03 Temperature & Humidity Sensor
void readXYMD03() {
  bool success = xymd03.update();
  
  if (success) {
    xymd03_connected = true;
    xymd03_temperature = xymd03.getTemperature();
    xymd03_humidity = xymd03.getHumidity();
  } else {
    xymd03_connected = false;
    simulateXYMD03();
  }
}

// Switch between display pages
void switchPage() {
  currentPage++;
  if (currentPage >= MAX_PAGES) {
    currentPage = 0;
  }
  
  // Show brief feedback flash
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 24);
  display.print("PAGE ");
  display.print(currentPage + 1);
  display.display();
  delay(150);
  
  // Update display immediately
  updateDisplay();
  lastDisplayUpdate = millis(); // Reset display timer
}

// Main display page (Page 0)
void updateDisplayPage0() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ========== HEADER ==========
  display.setTextSize(1);
  display.setCursor(2, 0);
  display.print("ESP32 Farm");
  display.setCursor(108, 0);
  display.print("1/2"); // Page indicator

  // ========== TEMPERATURE DISPLAY ==========
  display.setTextSize(1);
  display.setCursor(2, 13);
  display.print("Temp: ");
  display.print(currentTemperature, 1);
  display.print(" C");
  
  // Sensor status indicator
  if (!sensorConnected) {
    display.print(" [SIM]");
  } else {
    display.print(" [OK]");
  }
  
  // ========== SWITCH STATUS ==========
  display.setCursor(2, 25);
  display.print("SW:");
  display.print("U["); display.print(sw3.isPressed() ? "X" : " "); display.print("]");
  display.print("D["); display.print(sw2.isPressed() ? "X" : " "); display.print("]");
  display.print("S["); display.print(sw1.isPressed() ? "X" : " "); display.print("]");
  
  // ========== RELAY STATUS ==========
  display.setCursor(2, 35);
  display.print("RL:");
  display.print("F["); display.print(relayFan.getState() ? "X" : " "); display.print("]");
  display.print("P["); display.print(relayPump.getState() ? "X" : " "); display.print("]");
  display.print("H["); display.print(relayHeater.getState() ? "X" : " "); display.print("]");
  
  // ========== ISO INPUT STATUS ==========
  display.setCursor(2, 45);
  display.print("ISO:");
  display.print("T1[");
  display.print(iso1.isActive() ? "DRY" : "OK");
  display.print("] T2[");
  display.print(iso2.isActive() ? "FUL" : "OK");
  display.print("]");

  // ========== BOTTOM STATUS BAR (WIFI) ==========
  if (wifiConnected) {
    display.setCursor(2, 56);
    display.print("IP:");
    display.print(ipAddress);
  } else {
    display.setCursor(2, 56);
    display.print("WiFi: OFF");
  }
  
  display.display();
}

// XY-MD03 display page (Page 1)
void updateDisplayPage1() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ========== HEADER ==========
  display.setTextSize(1);
  display.setCursor(2, 0);
  display.print("XY-MD03");
  display.setCursor(100, 0);
  display.print("2/2"); // Page indicator

  // ========== SENSOR STATUS ==========
  display.setCursor(2, 13);
  display.print("Status: ");
  if (xymd03_connected) {
    display.print("Connected");
  } else {
    display.print("SIM Mode");
  }

  // ========== TEMPERATURE ==========
  display.setTextSize(1);
  display.setCursor(2, 26);
  display.print("Temperature: ");
  display.print(xymd03_temperature, 1);
  display.print(" C");

  // ========== HUMIDITY ==========
  display.setCursor(2, 37);
  display.print("Humidity:    ");
  display.print(xymd03_humidity, 1);
  display.print(" %");

  // ========== INFO ==========
  display.setCursor(2, 48);
  display.print("Modbus RS485");

  // ========== INSTRUCTION ==========
  display.setCursor(2, 56);
  display.print("SW1=Next Page");
  
  display.display();
}

// Main update display function - route to correct page
void updateDisplay() {
  switch (currentPage) {
    case 0:
      updateDisplayPage0();
      break;
    case 1:
      updateDisplayPage1();
      break;
    default:
      currentPage = 0;
      updateDisplayPage0();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
  }
  
  // Check for WiFi reset button (SW1) at startup
  checkWiFiResetButton();
  
  // Setup WiFi
  setupWiFi();
  
  // Show welcome with IP
  showWelcome();

  // Initialize DS18B20 Temperature Sensor
  tempSensor.begin();
  Serial.println("DS18B20 Temperature Sensor Initialized on GPIO14");
  Serial.print("Found ");
  Serial.print(tempSensor.getDeviceCount());
  Serial.println(" device(s).");
  
  // Initial temperature reading
  readTemperature();

  // Note: XY-MD03 uses Serial0 (RS485) - ต้องสลับ switch เป็นโหมด RS485
  // เมื่อใช้ XY-MD03 Serial Monitor จะไม่ทำงาน
  // Initialize XY-MD03 Temperature & Humidity Sensor (Modbus RTU)
  // Comment out these lines if not using XY-MD03 or using USB Serial Monitor
  Serial.end(); // ปิด Serial Monitor mode
  xymd03.begin(9600); // เริ่มต้น Modbus RTU mode
  delay(100);
  // Initial XY-MD03 reading
  readXYMD03();

  // Initialize switches
  sw1.begin();
  sw2.begin();
  sw3.begin();

  // Initialize relays
  relayFan.begin();
  relayPump.begin();
  relayHeater.begin();

  // Initialize isolated inputs
  iso1.begin();
  iso2.begin();

  // Register ISO callbacks
  iso1.onActive(onIso1Active);
  iso1.onInactive(onIso1Inactive);
  iso2.onActive(onIso2Active);
  iso2.onInactive(onIso2Inactive);

  // Register click callbacks
  sw1.onClick(onSw1Click);
  sw2.onClick(onSw2Click);
  sw3.onClick(onSw3Click);
}

void loop() {
  // Poll switches (debounce and edge detection handled by class)
  sw1.update();
  sw2.update();
  sw3.update();

  // Poll isolated inputs
  iso1.update();
  iso2.update();

  // Note: Serial control is disabled when using XY-MD03 (Serial0 used for Modbus)
  // Serial control: press keys to toggle relays (only works in USB mode)
  // if (Serial.available()) {
  //   char c = (char)Serial.read();
  //   switch (c) {
  //     case 'f': case 'F': toggleFan(); break;
  //     case 'p': case 'P': togglePump(); break;
  //     case 'h': case 'H': toggleHeater(); break;
  //     case '1': toggleFan(); break;
  //     case '2': togglePump(); break;
  //     case '3': toggleHeater(); break;
  //     default: break;
  //   }
  // }

  delay(10);

  // Update XY-MD03 reading at interval
  if (millis() - lastXYMD03Update >= XYMD03_UPDATE_INTERVAL) {
    lastXYMD03Update = millis();
    readXYMD03();
  }

  // Update temperature reading at interval
  if (millis() - lastTempUpdate >= TEMP_UPDATE_INTERVAL) {
    lastTempUpdate = millis();
    readTemperature();
  }

  // Update display at interval
  if (millis() - lastDisplayUpdate >= DISPLAY_INTERVAL) {
    lastDisplayUpdate = millis();
    updateDisplay();
  }
}

