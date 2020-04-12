#include <MQ135.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

//Wifi setup
const char* ssid="YOUR_WIFI_SSID";
const char* pswd="YOUR_WIFI_PASSWORD";

//MQTT Server
const char* mqtt_server="YOUR MQTT Broker Server";
#define mqtt_port 1883

//timer
long now=millis();
long lastMeasure=0;

//define analog pin
#define ANALOGPIN A0
#define RZERO 206.85
float rzero, ppm;
MQ135 gasSensor =MQ135(ANALOGPIN);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi(){
  delay(10);
  WiFi.begin(ssid,pswd);
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_connect(){
  Serial.println("Attempting MQTT Connection");
  if(client.connect("ESP8266Client")){
    Serial.println("MQTT Server Connected");
  }
  else{
    Serial.print("MQTT Connection failed, rc=");
    Serial.print(client.state());
    Serial.println("Reconnect in 5s");
    delay(5000);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(500);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  
  rzero = gasSensor.getRZero();
  delay(3000);
  Serial.print("MQ135 RZERO Calibration Value: ");
  Serial.println(rzero);
  
}

void loop() {
  if(!client.connected()){
    mqtt_connect();
  }

  if(!client.loop()){
    client.connect("ESpClient8266");
  }

  now = millis();
  if(now-lastMeasure > 5000){
    ppm = gasSensor.getPPM();

    if(isnan(ppm)){
      Serial.println("Failed to read MQ135 Sensor");
      client.publish("/mq135/ppm","Failed to read sensor");
    }


    static char ppmTemp[7];
    dtostrf(ppm,6,2,ppmTemp);
    client.publish("/mq135/ppm",ppmTemp);

    Serial.println("CO2 ppm value: ");
    Serial.println(ppm);

    delay(3000);
  }

}
