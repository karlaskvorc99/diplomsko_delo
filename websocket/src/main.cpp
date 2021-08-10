#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "DHT.h"
#include <string>
#include <iostream>

//dht senzor
#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

//bmp senzor
Adafruit_BMP280 bmp; // I2C Interface

//poverilnice za omrezje
const char* ssid = "PRO-W";
const char* password = "43omega21";

//deklaracija spremenljivk za vrednosti z senzorji
float h;
float t;
float f;
float hif;
float hic;

float tp;
float p;
float a;

//objekt na portu 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//html koda za spletno stran
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
    <meta charset="utf-8">
    <title>ESP Web Server</title>
    <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.1.0/css/bootstrap.min.css" integrity="sha384-9gVQ4dYFwwWSjIDZnLEWnxCjeSWFphJiwGPXr1jddIhOegiu1FwO5qRGvFXOdJZ4" crossorigin="anonymous">

    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
<style>
    body{
        background-color: #f7f4ea;
    }
    .my_class{
        background-color: #ded9e2;
        border:1px solid #c0b9dd;
    }
</style>
</head>
<body>
    <div class="row">
        <div class="col-md-4 col-sm-2">
        </div>
        <div class="form-group text-center col-md-4 col-sm-8 ">
            <p>DHT SENZOR</p>
        </div>
        <div class="col-md-4 col-sm-2">
        </div>
    </div>
    <div class="row">
        <div class="col-md-4 col-sm-2">
        </div>
        <div class="form-group text-center col-md-4 col-sm-8 my_class">
        <p class="temp">Temperature (°C): <span id="temp">%TEMP%</span></p>
        </div>
        <div class="col-md-4 col-sm-2">
        </div>
    </div>
  
    <div class="row">
        <div class="col-md-4 col-sm-2">
        </div>
        <div class="form-group text-center col-md-4 col-sm-8 my_class">
            <p class="tempt">Temperature (°F): <span id="tempt">%TEMPT%</span></p>
        </div>
        <div class="col-md-4 col-sm-2">
        </div>
    </div>
  
    <div class="row">
        <div class="col-md-4 col-sm-2">
        </div>
        <div class="form-group text-center col-md-4 col-sm-8 my_class">
            <p class="hum">Humidity: <span id="hum">%HUM%</span></p>
        </div>
        <div class="col-md-4 col-sm-2">
        </div>
    </div>
  
    <div class="row">
        <div class="col-md-4 col-sm-2">
        </div>
        <div class="form-group text-center col-md-4 col-sm-8 my_class">
            <p class="hic">Heat index (°C): <span id="hic">%HIC%</span></p>
        </div>
        <div class="col-md-4 col-sm-2">
        </div>
    </div>
    
    <div class="row">
        <div class="col-md-4 col-sm-2">
        </div>
        <div class="form-group text-center col-md-4 col-sm-8 my_class">
            <p class="hif">Heat index (°F): <span id="hif">%HIF%</span></p>
        </div>
        <div class="col-md-4 col-sm-2">
        </div>
    </div>
    <div class="row">
        <div class="col-md-4 col-sm-2">
        </div>
        <div class="form-group text-center col-md-4 col-sm-8 ">
            <p>BMP280 SENZOR</p>
        </div>
        <div class="col-md-4 col-sm-2">
        </div>
    </div>
    
    <div class="row">
        <div class="col-md-4 col-sm-2">
        </div>
        <div class="form-group text-center col-md-4 col-sm-8 my_class">
        <p class="temp2">Temperature (°C): <span id="temp2">%TEMP2%</span></p>
        </div>
        <div class="col-md-4 col-sm-2">
        </div>
    </div>
    
    <div class="row">
        <div class="col-md-4 col-sm-2">
        </div>
        <div class="form-group text-center col-md-4 col-sm-8 my_class">
            <p class="pres">Pressure (hPa): <span id="pres">%PRES%</span></p>
        </div>
        <div class="col-md-4 col-sm-2">
        </div>
    </div>
    
    <div class="row">
        <div class="col-md-4 col-sm-2">
        </div>
        <div class="form-group text-center col-md-4 col-sm-8 my_class">
            <p class="alti">Altitude (m): <span id="alti">%ALTI%</span></p>
        </div>
        <div class="col-md-4 col-sm-2">
        </div>
    </div>
    
    <div class="row">
        <div class="col-md-4 col-sm-2">
        </div>
        <div class="form-group text-center col-md-4 col-sm-8 my_class">
            <p><button id="button" class="button">REFRESH</button></p>
        </div>
        <div class="col-md-4 col-sm-2">
        </div>
    </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var data = event.data;

    var temp;
    var hum;
    var pres;
    var alti;
    var tempt;
    var hic;
    var hif;
    var temp2;

    var num=0;
    var word = "";
    
    for(var i=0;i<data.length;i++){
      if(data[i] == ","){
        num = num + 1;
        if(num == 1){
          temp = word;
        }
        else if (num == 2){
          pres = word;
        }
        else if (num == 3){
          hum = word;
        }
        else if (num == 4){
          alti = word;
        }
        else if (num == 5){
          hif = word;
        }
        else if(num == 6){
          hic = word;
        }
        else if(num == 7){
          tempt = word;
        }
        else {
          temp2 = word;
        }
        word="";
      }
      else{
        word = word + data[i];
      }
    }
    temp2 = word;

    document.getElementById('temp').innerHTML = temp;
    document.getElementById('pres').innerHTML = pres;
    document.getElementById('hum').innerHTML = hum;
    document.getElementById('tempt').innerHTML = tempt;
    document.getElementById('hif').innerHTML = hif;
    document.getElementById('hic').innerHTML = hic;
    document.getElementById('alti').innerHTML = alti;
    document.getElementById('temp2').innerHTML = temp2;

  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }
</script>
</body>
</html>
)rawliteral";

//funkcija s katerom posilamo nove vrednosti z senzorji vsem klijentima
void notifyClients() {
  String odg = "";
  odg = String(t) + String(",") + String(p) + String(",") + String(h)+ String(",") + String(a)+ String(",") + String(hif)+ String(",") + String(hic)+ String(",") + String(f)+ String(",") + String(tp);
  ws.textAll(odg);
}

//funkcija s katerom obvladamo sporocila kot so pritisk klijenta na gomb za osvezenje vrednosti
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    //ce smo kot klijent pritisnili gomb potem poklicemo funkcijo s katerom se enkrat poslemo nove vrednosti
    if (strcmp((char*)data, "toggle") == 0) {
      notifyClients();
    }
  }
}

//funckija s katerom gledamo kdo se vse od klijenti povezal ali pa prekinil povezavo 
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

//funckija s katerom zacnemo web socket
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

//funciija s katerom obvladamo placeholdere kot se stranica refresha
String processor(const String& var){
  Serial.println(var);
  if(var == "TEMP"){
    return String(t);
  }
  else if (var == "HUM"){
    return String(h);
  }
  else if (var == "PRES"){
    return String(p);
  }
  else if (var == "TEMPT"){
    return String(f);
  }
  else if (var == "ALTI"){
    return String(a);
  }
  else if (var == "HIF"){
    return String(hif);
  }
    else if (var == "HIC"){
    return String(hic);
  }
  else if (var == "TEMP2"){
    return String(tp);
  }
  return String();
}

//funkcija za zacetek, senzorji, postavitev servera, socket
void setup(){
  Serial.begin(115200);

  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
  dht.begin();
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */  

  
  //wifi povezovanje
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  //izpisemo adreso
  Serial.println(WiFi.localIP());

  initWebSocket();

  //postavitev rute 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  //zazenemo server
  server.begin();
}

void loop() {
  ws.cleanupClients();

  delay(2000);

    //branje vrednosti z senzorji

    //DHT11 INFO
    h = dht.readHumidity();
    t = dht.readTemperature();
    f = dht.readTemperature(true);

    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    hif = dht.computeHeatIndex(f, h);
    hic = dht.computeHeatIndex(t, h, false);

    //DHT11 PRINT
    /*
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.print(f);
    Serial.print(F("°F  Heat index: "));
    Serial.print(hic);
    Serial.print(F("°C "));
    Serial.print(hif);
    Serial.println(F("°F"));*/

    //BMP280 INFO
    tp = bmp.readTemperature();
    p = bmp.readPressure()/100;
    a = bmp.readAltitude(1013.66);

    //BMP280 PRINT
    /*Serial.print(F("Temperature: "));
    Serial.print(tp);
    Serial.print(F(" *C  Pressure: "));
    Serial.print(p);
    Serial.print(F(" hPa Approx altitude:"));
    Serial.print(a);
    Serial.println(" m"); */
}
