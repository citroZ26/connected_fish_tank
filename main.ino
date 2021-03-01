#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define ONE_WIRE_BUS D4
#define TEMPERATURE_PRECISION 10

const char* ssid = "ssid";
const char* password = "password";
const char* mqttServer = "mqttServer";
const int mqttPort = mqttPort; //1883
const long utcOffsetInSeconds = 3600;

WiFiClient espClient;
PubSubClient client(espClient);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = { 0x28,  0xDF,  0x38,  0x79,  0xA2,  0x19,  0x3,  0x7A };
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

long now = millis();
long lastMeasure = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("room/temperature");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

float getTemperatureEnC(DeviceAddress deviceAddress) {
  return sensors.getTempC(deviceAddress);
}

void setup() {  
  Serial.begin(115200);
  delay(10);

  pinMode(16,OUTPUT);
  pinMode(5,OUTPUT);
  
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  timeClient.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");

  now = millis();
  if (now - lastMeasure > 30000) {
    lastMeasure = now;
    char temperatureTemp[7];
    dtostrf(getTemperatureEnC(insideThermometer),6,2,temperatureTemp);
    client.publish("room/temperature", temperatureTemp);

    //if(temperatureTemp < 23){digitalWrite(16,1);} //D0
    //if(temperatureTemp > 26){digitalWrite(5,1);} // D1
    
  }
  /*timeClient.update();
  Serial.print(timeClient.getDay());
  if(timeClient.getDay() == 0){
    Serial.println("Changement d'eau !");
    digitalWrite(16,1);
    digitalWrite(5,1);
  }*/
  delay(1000);
} 
