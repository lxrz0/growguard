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
// const char* mqtt_svr = "23afd78a350347c689a20e4620b72ceb.s2.eu.hivemq.cloud";
const char* mqtt_svr = "test.mosquitto.org";
// const char* mqtt_username = "cl586";
// const char* mqtt_pwd = "cEu9NQfrRkvUmdYw6ZShn2";
const char* mqtt_publish_topic = "sensors/light";
const int mqtt_port = 1883;

/** wifi connection client init **/
WiFiClient wifiClient;
PubSubClient client(wifiClient);

/** sensor initialisation **/
BH1750 LightSensor;

/** certificate **/
const char* ca_cert = \
"-----BEGIN CERTIFICATE-----\n"
"MIIDzzCCAregAwIBAgIUCfESdE5OY1BYe3i34D5jpxgPQnUwDQYJKoZIhvcNAQEL\n"
"BQAwdzELMAkGA1UEBhMCR0IxEzARBgNVBAgMClNvbWUtU3RhdGUxDzANBgNVBAcM\n"
"BmxvbmRvbjEhMB8GA1UECgwYY2w1ODYgdW5pdmVyc2l0eSBvZiBrZW50MR8wHQYJ\n"
"KoZIhvcNAQkBFhBjbDU4NkBrZW50LmFjLnVrMB4XDTIzMTExOTE4MDAyNFoXDTI0\n"
"MTExODE4MDAyNFowdzELMAkGA1UEBhMCR0IxEzARBgNVBAgMClNvbWUtU3RhdGUx\n"
"DzANBgNVBAcMBmxvbmRvbjEhMB8GA1UECgwYY2w1ODYgdW5pdmVyc2l0eSBvZiBr\n"
"ZW50MR8wHQYJKoZIhvcNAQkBFhBjbDU4NkBrZW50LmFjLnVrMIIBIjANBgkqhkiG\n"
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAryfMWs6d96vS99fAn8Q0l6EqgwH87mJ5yPQ+\n"
"DF4zt20KVJB4sXBsbZsNc633hfyyA5mGe7cyawYcBdA51aRi0sf2SvZJ8DDx6Sxl\n"
"lOyvgrBNTmSIHkgtEKjRlyxgSe+UFCQGxOIe1Nm1T0gEUj73BmnPPgUW+dGfU2fj\n"
"ZRFJU3UNkGdPXOCFev7mA8m3ITLaTPl1/OfpF7t9oaAxC/FaerSG3qk6v5M3aJep\n"
"/vaC/yv7Th7KZ5J0pU6B6clD1bVGtJyuC7+stBra+kKg2thTO0wqMDjqeIcaEF9o\n"
"bCsKs0MfQJCnUZ9IKuntFqjwqO2nOwSb04CROY2IQOluF1ysXQIDAQABo1MwUTAd\n"
"BgNVHQ4EFgQUy5wfnBKmgJBtI8QLqMdx+fKTvx8wHwYDVR0jBBgwFoAUy5wfnBKm\n"
"gJBtI8QLqMdx+fKTvx8wDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOC\n"
"AQEAGl0LIpjAjFnIYuaFw6cjVooRO+ww3NnB9thF64cjjL5EqAwQN8w7GEiMnRm+\n"
"39nSrZF7p2l01AOpZcaw0NDpo1CIXye9kclS3DjaMXTqyEX9+QqZUGxpVqzx+P7e\n"
"/yTsh+0ydk2u37v6q8ynLtNmcNZPHbXeZw7k7v6f9cIVoTS6El3gBYZ6rH/ieoyu\n"
"y/AU3H/3xBAsFm2Q6jew0SCdWMyOFO5Vhjx1PoZZNrtNuz9Vb/oo2/6yFiGfwqYy\n"
"mDFZlP7+V+w3zLeJX8CEVWevWL+lctFjfDTGWoLQqbdotGIpeCO1f9Cq4mgGhvrN\n"
"IQerkbCMjwWs5NgCpgLrHqt9wA==\n"
"-----END CERTIFICATE-----\n";


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

    if (client.connect(clientId.c_str())) {
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
  // wifiClient.setCACert(ca_cert);
  
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

  delay(500);
  float lux = LightSensor.readLightLevel();
  Serial.println(lux);
  client.publish("sensors/data", String (lux, 2).c_str(), true);
}
