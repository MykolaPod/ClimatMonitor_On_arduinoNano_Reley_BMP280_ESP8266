#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <PubSubClient.h>

#define DHTPIN 2
#define DHTTYPE DHT22
#define GetSensorsDataInterval 60000

String orgId = "";
String deviceType = "ControlUnit";
String deviceId = "";
String authToken = "";
#define bmp_preasure_topic "iot-2/evt/preasure/fmt/json"
#define bmp_temperature_topic "iot-2/evt/temperature/fmt/json"
#define device_mode_topic "iot-2/evt/device_mode/fmt/json"
#define subscribeCommandTopic "iot-2/cmd/temperature/fmt/json"
String server = orgId + ".messaging.internetofthings.ibmcloud.com";
#define WiFiSSID ""
#define WiFiPassword ""
String PostTemperatureCommand = "POST_TEMP";
String PostPreasureCommand = "POST_PREA";
String PostModeCommand = "POST_MODE";
String EndOftransmissionCommand = "EOT";
#define OkResponse "OK"

bool _isWiFiConnected = false;
String TemperatureDataToBeSend = "";
String PreasureDataToBeSend = "";
String ModeDataToBeSend = "";
WiFiClient wifiClient;

void callback(char* topic, byte* payload, unsigned int payloadLength);
PubSubClient mqttClient((char*)server.c_str(), 1883, callback, wifiClient);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  tryConnectToWifi(WiFiSSID, WiFiPassword);
  if (_isWiFiConnected) {
    connectToMqtt();
  }
  mqttClient.setCallback (callback);

}


void loop() {


  if (Serial.available()) {
    String dataFromArduino = Serial.readString();

    if (dataFromArduino == "AT\r\n") {
      Serial.println(OkResponse);
      return;
    }
    if (dataFromArduino == "IP\r\n") {
      Serial.println(WiFi.localIP());
      return;
    }
    if (dataFromArduino == "SSID\r\n") {
      Serial.println(WiFiSSID);
      return;
    }
    if (dataFromArduino == "WIFISTATUS\r\n") {
      Serial.println(WiFi.status());
      return;
    }
    if (dataFromArduino == "BROKERSTATUS\r\n") {
      Serial.println(mqttClient.connected());
      return;
    }
    if (dataFromArduino.indexOf(PostTemperatureCommand) > -1 && dataFromArduino.indexOf(EndOftransmissionCommand) > -1 ) {
      byte indexOfStartValue = dataFromArduino.indexOf(PostTemperatureCommand) + PostTemperatureCommand.length();
      byte indexOfEndValue = dataFromArduino.indexOf(EndOftransmissionCommand);
      TemperatureDataToBeSend = dataFromArduino.substring(indexOfStartValue, indexOfStartValue + (indexOfEndValue - indexOfStartValue));
      sendDataToBroker();
      return;
    }
    if (dataFromArduino.indexOf(PostPreasureCommand) > -1 && dataFromArduino.indexOf(EndOftransmissionCommand) > -1 ) {
      byte indexOfStartValue = dataFromArduino.indexOf(PostPreasureCommand) + PostPreasureCommand.length();
      byte indexOfEndValue = dataFromArduino.indexOf(EndOftransmissionCommand);
      PreasureDataToBeSend = dataFromArduino.substring(indexOfStartValue, indexOfStartValue + (indexOfEndValue - indexOfStartValue));
      sendDataToBroker();
      Serial.println(OkResponse);
      return;
    }
    if (dataFromArduino.indexOf(PostModeCommand) > -1 && dataFromArduino.indexOf(EndOftransmissionCommand) > -1 ) {
      byte indexOfStartValue = dataFromArduino.indexOf(PostModeCommand) + PostModeCommand.length();
      byte indexOfEndValue = dataFromArduino.indexOf(EndOftransmissionCommand);
      ModeDataToBeSend = dataFromArduino.substring(indexOfStartValue, indexOfStartValue + (indexOfEndValue - indexOfStartValue));
      sendDataToBroker();
      Serial.println(OkResponse);
      return;
    }

  }


  if (!_isWiFiConnected) {
    tryConnectToWifi(WiFiSSID, WiFiPassword);
  }
  if (_isWiFiConnected) {
    connectToMqtt();
  }
  mqttClient.loop();
}

void sendDataToBroker() {

  if (!mqttClient.connected()) {
    connectToMqtt();
  }
  if (!mqttClient.connected()) {
    return;
  }
  
  if (TemperatureDataToBeSend.length() > 0) {
    String payload_temp = "{\"d\":{\"temperature\":";
    payload_temp += TemperatureDataToBeSend;
    payload_temp += "}}";
    mqttClient.publish(bmp_temperature_topic, (char*) payload_temp.c_str());
    TemperatureDataToBeSend = "";
  }

  if (PreasureDataToBeSend.length() > 0) {
    String payload_preasure = "{\"d\":{\"preasure\":";;
    payload_preasure += PreasureDataToBeSend;
    payload_preasure += "}}";
    mqttClient.publish(bmp_preasure_topic, (char*) payload_preasure.c_str());
    PreasureDataToBeSend = "";
  }
 
  if (ModeDataToBeSend.length() > 0) {
    String payload_mode = "{\"d\":{\"device_mode\":";;
    payload_mode += ModeDataToBeSend;
    payload_mode += "}}";
    mqttClient.publish(device_mode_topic, (char*) payload_mode.c_str());
    ModeDataToBeSend = "";
  }

}

bool tryConnectToWifi(char* ssid, char* password) {
  if (_isWiFiConnected) {
    return true;
  }
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  _isWiFiConnected = WiFi.status() == WL_CONNECTED;
  if (!isWiFiConnected()) {
    delay(1000);
    _isWiFiConnected = WiFi.status() == WL_CONNECTED;
  }

  return _isWiFiConnected;
}

bool isWiFiConnected() {
  if (!mqttClient.connected()) {
    _isWiFiConnected = (WiFi.status() == WL_CONNECTED);
  }
  return _isWiFiConnected;
}

void connectToMqtt() {
  if (mqttClient.connected()) {
    return;
  }
  String clientName;

  clientName += "d:";
  clientName += orgId + ":";
  clientName += deviceType + ":";
  clientName += deviceId;

  mqttClient.connect((char*) clientName.c_str(), "use-token-auth", (char*) authToken.c_str());

  if (mqttClient.subscribe(subscribeCommandTopic)) {
    //Serial.println(" client subscribed OK");
  } else {
    Serial.println("client subscribed FAILED");
  }
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {
  Serial.print("gotMsg: invoked for topic: "); Serial.println(topic);

//  if (String(topic).indexOf(CMD_STATE) > 0) {
//    String cmd = "";
//    for (int i = 0; i < payloadLength; i++) {
//      cmd += (char)payload[i];
//    }
//    
//  } else {
//    Serial.print("gotMsg: unexpected topic: "); Serial.println(topic);
//  }
}


