#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "FastLED.h"

#ifndef STASSID
#define STASSID "The515"
#define STAPSK  "Bluemoon696!"
#endif

#define DATA_PIN    D7
#define LED_TYPE    WS2811
#define COLOR_ORDER BRG
#define NUM_LEDS    100



CRGB leds[NUM_LEDS];
const char* ssid = STASSID;
const char* password = STAPSK;


//Variables to track for functions
int brightness = 255;
int r = 0;
int g = 0;
int b = 0;
int r2 = 0;
int g2 = 0; 
int b2 = 0;
int storedVariableA =0;
bool pulseDir = false;
bool initialSet = false;


//Global variables for Wave Function, courtesy of bitluni
long sinTab[256];
int start;

ESP8266WebServer server(80);


enum LoopFunction{
  Solid, Wave, Chase, Pulse, Demo, Weave, Sparkle
};

LoopFunction loopFunction;

void setup(void) {
  pinMode(13, OUTPUT);
  digitalWrite(13, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  random16_set_seed(analogRead(D5));
  
  startWifi();
  startLed();
  startOTA();

  loopFunction = Solid;
  
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  
  server.on("/brightness", urlSetBrightness);
  server.on("/PulseOld", pulse);  
  server.on("/off", [](){
      r = 0;
      g = 0;
      b = 0;
      loopFunction = Solid;
      initialSet = false;
    server.send(200,"text/plain","Off");
  });

  server.on("/white", [](){
    server.send(200,"text/plain","White");
    loopFunction = Solid;
    r = 255;
    g = 255;
    b = 180;
    initialSet = false;
    
  });

  server.on("/random", [](){
    server.send(200,"text/plain","Random");
    loopFunction = Solid;
    r = random(255);
    g = random(255);
    b = random(120);
    initialSet = false;
  });

  server.on("/solid", [](){
    server.send(200,"text/plain", "Solid");
    loopFunction = Solid;
    initialSet = false;
  });

  
  server.on("/wave", [](){
    server.send(200,"text/plain", "Wave");
    loopFunction = Wave;
    initialSet = false;
  });

  
  server.on("/chase", [](){
    server.send(200,"text/plain", "Chase");
    loopFunction = Chase;
    initialSet = false;
  });

  
  server.on("/pulse", [](){
    server.send(200,"text/plain", "Pulse");
    loopFunction = Pulse;
    initialSet = false;
  });

  server.on("/demo", [](){
    server.send(200,"text/plain", "Demo");
    loopFunction = Demo;
    initialSet = false;
  });

  server.on("/weave", [](){
    server.send(200,"text/plain", "Weave");
    loopFunction = Weave;
    initialSet = false;
  });

  server.on("/sparkle", [](){
    server.send(200,"text/plain", "sparkle");
    loopFunction = Sparkle;
    initialSet = false;
  });


  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  ArduinoOTA.handle();
  server.handleClient();
  MDNS.update();
  handleEnum();
}

void startWifi(){
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (MDNS.begin("ZAQ147")) {
    Serial.println("MDNS responder started");
  }
}

void startOTA(){
   ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void startLed(){
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  ledSweep();
}

//Handlers
void handleRoot() {
  digitalWrite(13, 1);
  server.send(200, "text/plain", "solid,sparkle,weave,pulse,demo,chase,wave,random,white");
  digitalWrite(13, 0);
}

void handleNotFound() {
  digitalWrite(13, 1);
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
  }

void handleEnum(){
  switch(loopFunction){
    case Solid:
      SolidFunction();
    break;
    case Wave:
      WaveFunction();
    break;
    case Chase:
      ChaseFunction();
    break;
    case Pulse:
      PulseFunction();
    break;
    case Demo:
      DemoFunction();
     break;
     case Weave:
      WeaveFunction();
     break; 
     case Sparkle:
      SparkleFunction();
     break; 
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

void pulse(){
  server.send(200,"text/plain","Chase");
  while(1==1)
    {
      int r = random8();
      int g = random8();
      int b = random8();
      int r2 = random8();
      int g2 = random8();
      int b2 = random8();
      for(int dot=(NUM_LEDS-1) ; dot >=0 ; dot--)
      { 
        leds[dot] = CRGB(r,g,b);
        FastLED.show();
        delay(25);
      }
      for(int dot = 0;dot < NUM_LEDS; dot++)
      {
        leds[dot] = CRGB(r2,g2,b2);
        FastLED.show();
        delay(10);
      }
    }
}


 //Loop functions

void SolidFunction(){
  if(!initialSet){
    fill_solid(leds, NUM_LEDS, CRGB(r,g,b));
    delay(20);
    FastLED.show();
    delay(20);
    initialSet = true;
  }else{
    //do nothing 
  }
}


//Courtesy of bitluni
void WaveFunction(){
  if(!initialSet){
    for(int i = 0; i < 256; i++){
     sinTab[i] = sin(3.1415 / 128 * i) * 0x7fff + 0x8000;
     Serial.print(sinTab[i]);
    }
    start = millis();
    initialSet = true;
  }

  
  int j = ((millis() - start) / 63) & 255;
  int k = ((millis() - start) / 71) & 255;
  for(int i = 0; i < NUM_LEDS; i++)
  {
    long s = (sinTab[(i*3 + j) & 255] >> 8) * (sinTab[-(i*4 + k) & 255] >> 8);
    leds[i] = CRGB((r * s) >> 16, (g * s) >> 16, (b * s) >> 16);
  }
  FastLED.show();
}

void ChaseFunction(){
  if(!initialSet)
  {
     if(server.args()>0){
      storedVariableA = server.arg(0).toInt();
    }else{
      storedVariableA = 700;
    }
    initialSet = true;
  }
   for(int q = 0; q <3; q++){
     for(int i = 0; i<NUM_LEDS; i +=3){
       leds[i+q] = CRGB(r,g,b);
     }
     FastLED.show();
     delay(storedVariableA);
     for(int i =0; i <NUM_LEDS; i+=3){
       leds[i+q] = CRGB(0,0,0);
     }
  }
}

void PulseFunction(){
  if(!initialSet){
      r = random8();
      g = random8();
      b = random8();
      r2 = random8();
      g2 = random8();
      b2 = random8();
      storedVariableA = NUM_LEDS-1;
      pulseDir = true;
      initialSet = true;
  }
   if(pulseDir){ //Go forward
      leds[storedVariableA--] = CRGB(r,g,b);
     if(storedVariableA == 0) {pulseDir = !pulseDir;}
   }else{ //Go Backwards 
      leds[storedVariableA++] = CRGB(r2,g2,b2);
      if(storedVariableA == NUM_LEDS-1){initialSet = false;} //reset with new colors.
   }
   delay(20);
   FastLED.show();
  
}

void WeaveFunction(){
  if(!initialSet){
    if(server.args()>0){
      storedVariableA = server.arg(0).toInt();
    }else{
      storedVariableA = 4;
    }
    initialSet = true;
  }
  fadeToBlackBy( leds, NUM_LEDS, 20);
    byte dothue = 0;
  for( int i = 0; i < storedVariableA; i++) {
    leds[beatsin16(i+4,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
    
  }
  delay(20);
  FastLED.show();
}

//https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectRandomColorTwinkle
void SparkleFunction(){
  if(!initialSet)
  {
     if(server.args()>0){
      storedVariableA = server.arg(0).toInt();
    }else{
      storedVariableA = 120;
    }
    initialSet = true;
    int i;
    for(i =0; i<NUM_LEDS; i++){
      leds[i] = CRGB(0,0,0);
    }
    FastLED.show();
  }
  int loc = random(NUM_LEDS);
  leds[loc] = CRGB(r,g,b);
  FastLED.show();
  delay(storedVariableA);
  leds[loc] = CRGB(0,0,0);
}

void DemoFunction(){
  //test
}
