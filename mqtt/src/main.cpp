#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"

//dht senzor
#define DHTPIN 4   
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);

//bmp senzor
Adafruit_BMP280 bmp; // I2C

//poverilnice za omrezje
const char* ssid = "PRO-W";
const char* password = "43omega21";

//adresa mqtt brokera
const char* mqtt_server = "192.168.20.4";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//deklaracija spremenljivk za vrednosti z senzorji
float h = 0;
float t = 0;
float f = 0;
float hif = 0;
float hic = 0;
float tp = 0;
float p = 0;
float a = 0;

//definicija pina za prizganje led
const int ledPin = 2;

//funckija za uspesno povezovanje
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
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//funkcija s katerom obvladamo stanje nekaterega GPIO pina dobimo info za spremembo stanja
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  //sprememba stanja ledice
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}

//funkcija za zacetek, senzorji, led, omrežje
void setup() {
  Serial.begin(115200);
  if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
  dht.begin();
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
}

//funkcija za zacetek mqtt konekcije
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

//glavna zanka
void loop() {
  //ce klijent ni povezan, potem poskusa se enkrat
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    //branje z DHT senzorja
    h = dht.readHumidity();
    t = dht.readTemperature();
    f = dht.readTemperature(true);

    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    //publish-amo brane vrednosti v topice
    char string1[8];
    dtostrf(h, 1, 2, string1);
    client.publish("esp32/DHT_humidity", string1);

    char string2[8];
    dtostrf(t, 1, 2, string2);
    client.publish("esp32/DHT_temperature_c", string2);

    char string3[8];
    dtostrf(f, 1, 2, string3);
    client.publish("esp32/DHT_temperature_f", string3);

    hif = dht.computeHeatIndex(f, h);
    hic = dht.computeHeatIndex(t, h, false);

    char string4[8];
    dtostrf(hif, 1, 2, string4);
    client.publish("esp32/DHT_heat_index_f", string4);

    char string5[8];
    dtostrf(hic, 1, 2, string5);
    client.publish("esp32/DHT_heat_index_c", string5);

    //branje z BMP senzoja
    tp = bmp.readTemperature();
    p = bmp.readPressure()/100;
    a = bmp.readAltitude(1013.66);

    //publish-amo brane vrednosti
    char string6[8];
    dtostrf(tp, 1, 2, string6);
    client.publish("esp32/BMP_temperature_c", string6);

    char string7[8];
    dtostrf(p, 1, 2, string7);
    client.publish("esp32/BMP_pressure", string7);

    char string8[8];
    dtostrf(a, 1, 2, string8);
    client.publish("esp32/BMP_altitude", string8);
  }
}
