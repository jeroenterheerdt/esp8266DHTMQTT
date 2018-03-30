//WIFI
#include <ESP8266WiFi.h>
const char* ssid = "YOURSSID";
const char* password = "YOURPASSWORD";

//MQTT
#define mqtt_server "YOURMQTTSERVER"
#define mqtt_user "YOURMQTTUSER"
#define mqtt_password "YOURMQTTPASSWORD"
#define humidity_topic "your_topic/humidity"
#define temperature_topic "your_topic/temperature"
#define heatindex_topic "your_topic/heatindex"
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);

//DHT shield
const int DHTPin = D4;
const int16_t SENSORERRORVALUE = -99;
int16_t tempValue = SENSORERRORVALUE;
float oldTempValue = SENSORERRORVALUE;
#include "DHT.h"
#define DHTTYPE DHT11
DHT dht(DHTPin, DHTTYPE);

//vars
//minimal difference before reporting anything over mqtt
float diff = 0.1;
float hum = SENSORERRORVALUE;
float temp = SENSORERRORVALUE;
float hic = SENSORERRORVALUE;
float newhic = SENSORERRORVALUE;
  
boolean connectWIFI()
{
  Serial.println("Connecting to WIFI");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int count = 0;
  while (count < 20) {
    if (WiFi.status() == WL_CONNECTED) {
      return (true);
    }
    delay(500);
    count++;
  }
  Serial.println("Wifi Connection timed out.");
  return false;
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup() {
  Serial.begin(38400);
  Serial.println("Booting...");
  if (connectWIFI()) {
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
    //call watchdog to prevent sleeping
  ESP.wdtFeed();
  //connect to MQTT server
  client.setServer(mqtt_server,1883);
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

void loop() {
    //call watchdog to prevent sleeping
  ESP.wdtFeed();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  //GET THE TEMPERATURE, humidity, etc from DHT
  float newhum = dht.readHumidity();
  float newtemp = dht.readTemperature();
  if (isnan(newhum) || isnan(newtemp)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else {
    //we got something useful from the dht.
    //compute heat index
    newhic = dht.computeHeatIndex(newtemp, newhum, false);
  }
  Serial.print("Temperature: ");
  Serial.println(newtemp);
  Serial.print("Humidity: ");
  Serial.println(newhum);
  Serial.print("HeatIndex: ");
  Serial.println(newhic);

  if (checkBound(newtemp,temp,diff)) {
    temp = newtemp;
    Serial.println("Temperature difference large enough, reporting over MQTT");
    client.publish(temperature_topic,String(temp).c_str(),true);
  }
  if (checkBound(newhum,hum,diff)) {
    hum = newhum;
    Serial.println("Humidity difference large enough, reporting over MQTT");
    client.publish(humidity_topic,String(hum).c_str(),true);
  }
  if (checkBound(newhic,hic,diff)) {
    hic = newhic;
    Serial.println("Heat Index difference large enough, reporting over MQTT");
    client.publish(heatindex_topic,String(hic).c_str(),true);
  }
  //call watchdog to prevent sleeping
  ESP.wdtFeed();

  //delay 10s
  for (int i = 0; i < 11; i++) {
    delay(1000);
      //call watchdog to prevent sleeping
    ESP.wdtFeed();
  }
}
