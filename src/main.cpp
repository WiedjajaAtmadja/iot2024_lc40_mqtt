#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

#define MQTT_BROKER  "broker.emqx.io"
#define MQTT_TOPIC_SUBSCRIBE "binus/iot2024/esp32/cmd"
#define MQTT_TOPIC_PUBLISH   "binus/iot2024/esp32/data"

char g_szDeviceId[30];
Ticker tickerTimer;
WiFiClient espClient;
PubSubClient mqtt(espClient);
boolean mqttConnect();
void WiFi_Connect();
void onPublishMessage()
{
  char szMsg[50];
  static int nMsgCount=0;
  sprintf(szMsg, "Hello from %s - %d", g_szDeviceId, nMsgCount++);
  mqtt.publish(MQTT_TOPIC_PUBLISH, szMsg);
}

void onTimer()
{
  Serial.println("1 second passed");
  onPublishMessage();
} 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  Serial.println("Booting...");
  WiFi_Connect();
  mqttConnect();
  tickerTimer.attach(3, onTimer);
}

void loop() {

  mqtt.loop();
}

void WiFi_Connect()
{
  WiFi.mode(WIFI_STA);
  // WiFi.begin("hd3_1", "celab123");
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.write(payload, len);
  digitalWrite(2, payload[0]-'0');
  digitalWrite(3, payload[1]-'0');

  Serial.println();
}
boolean mqttConnect() {

  sprintf(g_szDeviceId, "esp32_%08X",(uint32_t)ESP.getEfuseMac());
  mqtt.setServer(MQTT_BROKER, 1883);
  mqtt.setCallback(mqttCallback);
  Serial.printf("Connecting to %s clientId: %s\n", MQTT_BROKER, g_szDeviceId);

  boolean fMqttConnected = false;
  for (int i=0; i<3 && !fMqttConnected; i++) {
    Serial.print("Connecting to mqtt broker...");
    fMqttConnected = mqtt.connect(g_szDeviceId);
    if (fMqttConnected == false) {
      Serial.print(" fail, rc=");
      Serial.println(mqtt.state());
      delay(1000);
    }
  }
  if (fMqttConnected)
  {
    Serial.println(" success");
    bool fResult = mqtt.subscribe(MQTT_TOPIC_SUBSCRIBE);
    Serial.printf("Subcribe topic: %s->%d \r\n", MQTT_TOPIC_SUBSCRIBE, fResult);
    // onPublishMessage();
  }
  return mqtt.connected();
}