#include <Wire.h>
#include <BH1750.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#define BAUD_RATE 9600

/** WiFi connection details **/
const char* ssid = "NETGEAR11";
const char* wifi_pwd = "rapidwind673";

/** mqtt broker details **/
const char* mqtt_svr = "23afd78a350347c689a20e4620b72ceb.s2.eu.hivemq.cloud";
// const char* mqtt_svr = "test.mosquitto.org";
const char* mqtt_username = "cl586";
const char* mqtt_pwd = "cEu9NQfrRkvUmdYw6ZShn2";
const char* mqtt_publish_topic = "sensors/light";
const int mqtt_port = 8883;

/** wifi connection client init **/
WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

/** sensor initialisation **/
BH1750 LightSensor;

/** certificate **/
const char* ca_cert = R"(
-----BEGIN CERTIFICATE-----
MIIFYDCCBEigAwIBAgIQQAF3ITfU6UK47naqPGQKtzANBgkqhkiG9w0BAQsFADA/
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT
DkRTVCBSb290IENBIFgzMB4XDTIxMDEyMDE5MTQwM1oXDTI0MDkzMDE4MTQwM1ow
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwggIiMA0GCSqGSIb3DQEB
AQUAA4ICDwAwggIKAoICAQCt6CRz9BQ385ueK1coHIe+3LffOJCMbjzmV6B493XC
ov71am72AE8o295ohmxEk7axY/0UEmu/H9LqMZshftEzPLpI9d1537O4/xLxIZpL
wYqGcWlKZmZsj348cL+tKSIG8+TA5oCu4kuPt5l+lAOf00eXfJlII1PoOK5PCm+D
LtFJV4yAdLbaL9A4jXsDcCEbdfIwPPqPrt3aY6vrFk/CjhFLfs8L6P+1dy70sntK
4EwSJQxwjQMpoOFTJOwT2e4ZvxCzSow/iaNhUd6shweU9GNx7C7ib1uYgeGJXDR5
bHbvO5BieebbpJovJsXQEOEO3tkQjhb7t/eo98flAgeYjzYIlefiN5YNNnWe+w5y
sR2bvAP5SQXYgd0FtCrWQemsAXaVCg/Y39W9Eh81LygXbNKYwagJZHduRze6zqxZ
Xmidf3LWicUGQSk+WT7dJvUkyRGnWqNMQB9GoZm1pzpRboY7nn1ypxIFeFntPlF4
FQsDj43QLwWyPntKHEtzBRL8xurgUBN8Q5N0s8p0544fAQjQMNRbcTa0B7rBMDBc
SLeCO5imfWCKoqMpgsy6vYMEG6KDA0Gh1gXxG8K28Kh8hjtGqEgqiNx2mna/H2ql
PRmP6zjzZN7IKw0KKP/32+IVQtQi0Cdd4Xn+GOdwiK1O5tmLOsbdJ1Fu/7xk9TND
TwIDAQABo4IBRjCCAUIwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYw
SwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5pZGVudHJ1
c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTEp7Gkeyxx
+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEEAYLfEwEB
ATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2VuY3J5cHQu
b3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0LmNvbS9E
U1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFHm0WeZ7tuXkAXOACIjIGlj26Ztu
MA0GCSqGSIb3DQEBCwUAA4IBAQAKcwBslm7/DlLQrt2M51oGrS+o44+/yQoDFVDC
5WxCu2+b9LRPwkSICHXM6webFGJueN7sJ7o5XPWioW5WlHAQU7G75K/QosMrAdSW
9MUgNTP52GE24HGNtLi1qoJFlcDyqSMo59ahy2cI2qBDLKobkx/J3vWraV0T9VuG
WCLKTVXkcGdtwlfFRjlBz4pYg1htmf5X6DYO8A4jqv2Il9DjXA6USbW1FzXSLr9O
he8Y4IWS6wY7bCkjCWDcRQJMEhg76fsO3txE+FiYruq9RUWhiF1myv4Q6W+CyBFC
Dfvp7OOGAN6dEOM4+qR9sdjoSYKEBpsr6GtPAQw4dy753ec5
-----END CERTIFICATE-----
)";


// track number of wifi connection attempts
unsigned short reconnection_count = 0;


void publishMessage (const char* topic, String payload, boolean retained) {}

/** callback method for receiving mqtt messages **/
void handleEvent(char* topic, byte* payload, unsigned int length) {
  Serial.printf("Message received on: %s", topic);
  Serial.println();

  for (int i=0; i<length;i++) {
    Serial.print( (char) payload[i]);
  }

  Serial.println();
}

/** setup and connect to wifi **/
void setupWifi () {
  delay(50);
  /** connect to wifi **/
  Serial.print("Connecting to wifi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wifi_pwd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.printf("connecting, attempt: %d", reconnection_count);
    Serial.println();
    reconnection_count++;
  }

  wifiClient.setCACert(ca_cert);

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

/** connect to MQTT broker **/
void connectMqtt () {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    Serial.println();
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_username, mqtt_pwd)) {
      Serial.printf("connected to: %s", mqtt_svr);
      Serial.println();

      // subscribe to topic
      client.subscribe("sensors/action");

      delay(100);
      Serial.print("Pushing a value now");

      client.publish("sensors/data", "hello world", true);
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.println(client.state());
      delay(4000);
    }
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  
  setupWifi();
  // wifiClient.setInsecure();
  
  
  // /** handle mqtt connection **/
  client.setServer(mqtt_svr, mqtt_port);
  client.setCallback(handleEvent);
  connectMqtt();

  /** initialise light sensor **/
  Wire.begin();
  LightSensor.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
  LightSensor.begin();

  Serial.println(F("BH1750 Test begin"));


}

void loop() {

  if (!client.connected()) connectMqtt();
  client.loop();

  delay(1000);
  float lux = LightSensor.readLightLevel();
  Serial.println(lux);
  // client.publish("sensors/data", String (lux, 2).c_str(), true);
}
