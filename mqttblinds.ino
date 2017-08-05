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


/************ WIFI and MQTT INFORMATION (CHANGE THESE FOR YOUR SETUP) ******************/
#define wifi_ssid "********" //enter your WIFI SSID
#define wifi_password "************" //enter your WIFI Password

#define mqtt_server "***.***.*.***" // Enter your MQTT server adderss or IP. I use my DuckDNS adddress (yourname.duckdns.org) in this field
#define mqtt_user "*******" //enter your MQTT username
#define mqtt_password "***********" //enter your password








WiFiClient espClient;
PubSubClient client(espClient);

Servo myservo;

int val;
int itsatrap = 0;


void setup() {
  Serial.begin(115200);

  setup_wifi();

  client.setServer(mqtt_server, 1883); //CHANGE PORT HERE IF NEEDED
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
    if (itsatrap == 0 && mytopic == "/blind/bc/command" && message.equals("ON")){  
      myservo.attach(D3);
      delay(500);
      myservo.write(90); 
      client.publish("/blind/bc/state", "ON");
      delay(1000);
      myservo.detach();
      }
    else if (mytopic == "/blind/bc/command" && message.equalsIgnoreCase("OFF")){
      myservo.attach(D3);
      delay(500);
      myservo.write(0);  
      client.publish("/blind/bc/state", "OFF");
      delay(1000);
      myservo.detach();
    }
    else if (mytopic == "/blind/bc/level"){
      myservo.attach(D3);
      delay(500);
      val = message.toInt(); //converts command to integer to be used for positional arrangement
      val = map (val, 0, 99, 0, 180);
      myservo.write(val);
      client.publish("/blind/bc/state", "ON");
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
    // Attempt to connect
  if (client.connect("ESPBlindstl666", mqtt_user, mqtt_password)) {
      Serial.println("connected");

      client.subscribe("/blind/bc/command");
      client.subscribe("/blind/bc/level");
      client.publish("/blind/bc/state", "OFF");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/* 
[In Home Assistant Configuration.yaml]
light:
  - platform: mqtt
    name: "Window Bottom Center"
    state_topic: "/blind/bc/state"
    command_topic: "/blind/bc/command"
    brightness_state_topic: "/blind/bc/state"
    brightness_command_topic: "/blind/bc/level"
    brightness_scale: 100
    qos: 0
    payload_on: "ON"
    payload_off: "OFF"
    optimistic: false
    retain: true
    
    
    Servo: FS90R from AliExpress
    
    Problems: 
    - ON and OFF rotate the servo the same way
    - Brightness is not rotating as it should
    
    */
