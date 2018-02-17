#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
/***************************************************************************/
/****************************** CONFIGURATION ******************************/
/***************************************************************************/
#define wifi_ssid "<Your SSID>" //enter your WIFI SSID
#define wifi_password "<Your Password>" //enter your WIFI Password

#define MQTT_Client "MQTT-Blind"
#define servo_pin D4

#define mqtt_server "<Your IP>" // Enter your MQTT Broker IP
#define mqtt_user "<Your User id>" //enter your MQTT username
#define mqtt_password "<Your Password>" //enter your password
#define mqtt_port 1883

#define mqtt_topic_payload "/" MQTT_Client "/payload/"
#define mqtt_topic_state "/" MQTT_Client "/state/"
#define mqtt_topic_brightness "/" MQTT_Client "/brightness/"

#define cmnd_on "ON"
#define cmnd_off "OFF"

#define servo_write_on 90
#define servo_write_off 0
/***************************************************************************/
/***************** DONT NEED TO CHANGE ANYTHING FROM HERE ******************/
/***************************************************************************/
WiFiClient espClient;
PubSubClient client(espClient);
Servo myservo;
int val;
int itsatrap = 0;

void setup() 
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);
    String mytopic(topic);
    if (itsatrap == 0 && mytopic == mqtt_topic_payload && message.equals(cmnd_on)){  
      myservo.attach(servo_pin);
      delay(500);
      myservo.write(servo_write_on); 
      client.publish(mqtt_topic_state, cmnd_on);
      delay(1000);
      myservo.detach();
      }
    else if (mytopic == mqtt_topic_payload && message.equalsIgnoreCase(cmnd_off)){
      myservo.attach(servo_pin);
      delay(500);
      myservo.write(servo_write_off);  
      client.publish(mqtt_topic_state, cmnd_off);
      delay(1000);
      myservo.detach();
    }
    else if (mytopic == mqtt_topic_brightness){
      myservo.attach(servo_pin);
      delay(500);
      val = message.toInt(); //converts command to integer to be used for positional arrangement
      val = map (val, 0, 99, 0, 180);
      myservo.write(val);
      client.publish(mqtt_topic_state, cmnd_on);
      delay(3000);
      myservo.detach();
      itsatrap = 1;
    }
    else{
        itsatrap = 0;
    }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = MQTT_Client;
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
  if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_payload);
      client.subscribe(mqtt_topic_brightness);
      client.publish(mqtt_topic_state, cmnd_off);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
