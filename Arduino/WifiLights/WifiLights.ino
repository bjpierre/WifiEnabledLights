#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "FastLED.h"

#ifndef STASSID
#define STASSID "The515"
#define STAPSK  "Bluemoon696!"
#endif

#define DATA_PIN    D7
#define LED_TYPE    WS2811
#define COLOR_ORDER BRG
#define NUM_LEDS    100

int brightness = 255;
CRGB leds[NUM_LEDS];
const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "Please leave my led alone");
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  
  startWifi();
  startLed();

  server.on("/", handleRoot);
  
  server.on("/brightness", urlSetBrightness);
  
  server.on("/off", [](){
    FastLED.clear();
    FastLED.show();
    server.send(200,"text/plain","Off");
    
  });

  server.on("/red",[](){
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    server.send(200,"text/plain","Red");
    FastLED.show();
  });

  server.on("/blue",[](){
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    server.send(200,"text/plain","Blue");
    FastLED.show();
  });

  server.on("/green", [](){
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    server.send(200,"text/plain","Green");
    FastLED.show();
  });

  server.on("/white", [](){
    fill_solid(leds, NUM_LEDS, CRGB::White);
    server.send(200,"text/plain","White");
    FastLED.show();
  });

  server.on("/rgb", urlRGB);

  server.on("/random", [](){
    fill_solid(leds, NUM_LEDS, CHSV(random8(),255,255));
    server.send(200,"text/plain","Random");
    FastLED.show();
  });

  server.on("/sweep", ledSweep);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}

void startWifi(){
   // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
}

void ledSweep(){
  server.send(200,"text/plain","Sweeping");
   
  int i =0, j =-4, k=-8;
  while(k<NUM_LEDS){
    if(++i<=NUM_LEDS){
      leds[i-1] = CRGB(0,0,0);
      leds[i] = CRGB::Red;
    }
    if(++j<=NUM_LEDS && j>=0){
      leds[j-1] = CRGB(0,0,0);
      leds[j] = CRGB::Blue;
    }
    if(++k<=NUM_LEDS && k>=0){
      leds[k-1] = CRGB(0,0,0);
      leds[k] = CRGB::Green;
    }
    FastLED.show();
    delay(20);
  }
}

void urlSetBrightness(){
  if(server.args()==1){
    brightness = server.arg(0).toInt();
    FastLED.setBrightness(brightness);
    server.send(200,"text/plain","Set Brightness");
    FastLED.show();
    }else server.send(200,"text/plain", "Bad parameters");
  
}

void urlRGB(){
  int r,g,b;
  if(server.args()==3){
    r=server.arg("r").toInt();
    g=server.arg("g").toInt();
    b=server.arg("b").toInt(); 
    fill_solid(leds, NUM_LEDS, CRGB(r,g,b));
    FastLED.show();
    server.send(200,"text/plain","Set color");
  }else server.send(200,"text/plain", "Bad parameters");
}

void startLed(){
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  FastLED.show();
  ledSweep();
}
