#include <WiFi.h>
#include <PubSubClient.h>
#include "settings.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "heltec.h"




#define RXD2 13
#define TXD2 12

WiFiClient espClient;
PubSubClient client(espClient);
void callback(char* topic, byte* payload, unsigned int length);

StaticJsonDocument<1000> out;

void status(String stat)
{
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, stat);
    Heltec.display->display();

}

const int MESSAGE_COUNT_THRESHOLD = 100;
unsigned long last_message_time = 0;
int message_count = 0;

void setup() {
  Serial.begin(115200);
   Serial2.begin(115200, SERIAL_8N1, RXD2,TXD2);

  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  Heltec.display->setFont(ArialMT_Plain_10);


  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    status("Connecto to WiFi...");
  }

  Serial.println("Connected to WiFi");
  status("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();



}


void reconnect()
{
    while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    status("Connecting to MQTT...");
    if (client.connect(mqtt_name)) {
      Serial.println("Connected to MQTT broker");
      status("Connected to MQTT");

      client.subscribe(mqtt_recv);
      //Serial.print("Subscribed to topic: ");

    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }

}

float a2;
float a3;
float a4;
float a5;
int counter = 0;



void dispV(float aV, String lV, int y )
{
    Heltec.display->drawString(0, y, lV);
    Heltec.display->drawString(20, y, String(aV,3));
    Heltec.display->drawString(50, y, "V");
}

void loop() {
  if (Serial2.available() > 0) {
    // Read the incoming message
    String message = Serial2.readStringUntil('\n');
    message.trim();
    Serial.println(message);
    // Check if 1 second has passed since the last message was sent
      if (millis() - last_message_time >= 100) {
        // Publish the message to the MQTT broker
        DeserializationError error = deserializeJson(out, message);
        if (out.containsKey("a2"))    a2 = out["a2"];
        if (out.containsKey("a3"))    a3 = out["a3"];
        if (out.containsKey("a4"))    a4 = out["a4"];
        if (out.containsKey("a5"))    a5 = out["a5"];


        Heltec.display->clear();
        Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
        dispV(a2,"a2:",0);
        dispV(a3,"a3:",10);
        dispV(a4,"a4:",20);
        dispV(a5,"a5:",30);
        Heltec.display->display();

        counter +=1;
        if (counter > 9)
        {
            client.publish(mqtt_send, message.c_str());
            counter = 0;
        }

        // Reset the last message time
        last_message_time = millis();
      }
    }
  

  // Check for incoming MQTT messages
  if (!client.connected()) reconnect();
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    Serial2.print((char)payload[i]);

  }
  
  Serial.println();
  Serial2.println();
  

}

