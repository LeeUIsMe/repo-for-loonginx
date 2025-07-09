#include <WiFi.h>
#include <ArduinoJson.h>

// 配置区（根据实际修改）
const char* WIFI_SSID = "Tenda";
const char* WIFI_PASS = "lusiyi125";
const char* SERVER_IP = "192.168.0.199";
const uint16_t SERVER_PORT = 1234;
const uint32_t SEND_INTERVAL_MS = 2000;

// 通信协议版本
#define PROTOCOL_VERSION 1

// 全局对象
WiFiClient tcpClient;
uint32_t lastDataSendTime = 0;

#pragma pack(push, 1)
typedef struct {
  float temperature;  // 单位：℃
  float humidity;     // 单位：%
  uint16_t light;     // 单位：lux
  int8_t rssi;        // 信号强度
} SensorData;
#pragma pack(pop)

void setup() {
  Serial.begin(115200);
  initWiFi();
  initSensors();
}

void loop() {
  maintainConnection();
  
  if (millis() - lastDataSendTime >= SEND_INTERVAL_MS) {
    SensorData data = readSensors();
    if (!sendSensorData(data)) {
      Serial.println("[WARN] 数据发送失败，将重试");
    }
    lastDataSendTime = millis();
  }
}

// WiFi初始化
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  Serial.printf("[WiFi] 正在连接 %s...", WIFI_SSID);
  for (int i = 0; i < 15 && WiFi.status() != WL_CONNECTED; ++i) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WiFi] 连接成功! IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n[WiFi] 连接失败!");
  }
}

// 传感器初始化
void initSensors() {
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  randomSeed(analogRead(36));
}

// 连接维护
void maintainConnection() {
  static uint32_t lastCheckTime = 0;
  
  if (millis() - lastCheckTime > 5000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WiFi] 连接断开，尝试重连...");
      WiFi.reconnect();
    }
    
    if (!tcpClient.connected()) {
      connectToServer();
    }
    
    lastCheckTime = millis();
  }
}

// 服务器连接
void connectToServer() {
  tcpClient.stop();
  Serial.printf("[NET] 正在连接服务器 %s:%d...\n", SERVER_IP, SERVER_PORT);
  
  if (tcpClient.connect(SERVER_IP, SERVER_PORT, 3000)) {
    Serial.println("[NET] 服务器连接成功");
    sendHandshake();
  } else {
    Serial.println("[NET] 服务器连接失败");
  }
}

// 握手协议
void sendHandshake() {
  DynamicJsonDocument doc(64);
  doc["ver"] = PROTOCOL_VERSION;
  doc["type"] = "HELLO";
  
  String output;
  serializeJson(doc, output);
  tcpClient.println(output);
}

// 传感器读取
SensorData readSensors() {
  SensorData data;
  
  // 模拟数据（实际项目替换为真实传感器读取）
  data.temperature = 25.5f + (random(61) - 30) / 100.0f;
  data.humidity = 85.0f + (random(101) - 50) / 10.0f;
  data.light = ((4095 - analogRead(35)) * 1200) / 4095;
  data.rssi = WiFi.RSSI();
  
  return data;
}

// 数据发送
bool sendSensorData(const SensorData &data) {
  if (!tcpClient.connected()) {
    return false;
  }

  DynamicJsonDocument doc(256);
  doc["ver"] = PROTOCOL_VERSION;
  doc["type"] = "DATA";
  doc["temp"] = round(data.temperature * 10) / 10.0;
  doc["humi"] = round(data.humidity * 10) / 10.0;
  doc["light"] = data.light;
  doc["rssi"] = data.rssi;
  doc["time"] = millis();

  String jsonStr;
  serializeJson(doc, jsonStr);
  
  tcpClient.println(jsonStr);
  Serial.printf("[NET] 发送数据: %s\n", jsonStr.c_str());

  // 等待确认
  uint32_t start = millis();
  while (millis() - start < 1000) {
    if (tcpClient.available()) {
      String resp = tcpClient.readStringUntil('\n');
      if (resp.indexOf("ACK") >= 0) {
        return true;
      }
    }
    delay(10);
  }
  
  return false;
}