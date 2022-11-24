#define BLYNK_PRINT Serial
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <GravityTDS.h>
#include <DHT.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
SoftwareSerial EspSerial(0, 1); // RX, TX
char auth[] = "5-sU8PFFZtnzSevjFcqmu0gjZP3eoSbj";
char ssid[] = "Lupikk";
char pass[] = "grimmjow";
#define TRIGGERPIN 10
#define ECHOPIN    11
#define DHTPIN 12
#define phsensor A0
#define ldr A2
#define tds A1
#define ESP8266_BAUD 9600 // ESP8266 baud rate:
#define tank 4
#define ppma 3
#define ppmb 4
#define phup 5
#define phdown 6
#define growl 7
#define fan 8
#define pump 9
#define DHTTYPE DHT22
ESP8266 wifi(&Serial);
WidgetLCD lcd1(V1);
WidgetLCD lcd2(V2);
float vol = 0;
float phm = 0;
float tdsen = 0;
int luxin = 0;
int fanb = 0;
unsigned long int avgValue;
float b;
int buf[10], temp;
float pH_value;
float pHVol;
GravityTDS gravityTds;
float temperature = 25,tdsValue = 0;
float lux=0.00,ADC_value=0.0048828125,LDR_value;
DHT dht(DHTPIN, DHTTYPE);
void setup()
{
  Serial.begin(9600);
  Serial.begin(ESP8266_BAUD);
  delay(10);
  pinMode(TRIGGERPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);
  pinMode(tank,OUTPUT);
  pinMode(ppma,OUTPUT);
  pinMode(ppmb,OUTPUT);
  pinMode(phup,OUTPUT);
  pinMode(phdown,OUTPUT);
  pinMode(growl,OUTPUT);
  pinMode(fan,OUTPUT);
  pinMode(pump,OUTPUT);
  pinMode (phsensor, INPUT);
  gravityTds.setPin(tds);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();
  dht.begin();  
  pinMode(LED_BUILTIN, OUTPUT);
  Blynk.begin(auth, wifi, ssid, pass); 
}
BlynkTimer timer;
void dstream(){
  Blynk.virtualWrite(V5, millis() / 1000);
  Blynk.virtualWrite(V7, millis() / 1000);
  Blynk.virtualWrite(V9, millis() / 1000);
  Blynk.virtualWrite(V11, millis() / 1000);
  Blynk.virtualWrite(V12, millis() / 1000);
}
BLYNK_WRITE(V4){
  vol = param.asFloat();  
}
void waterlevel(){
  long duration, distance;
  lcd1.print(0,0,"Min Volume is 1L");
  digitalWrite(TRIGGERPIN, LOW);  
  delayMicroseconds(3); 
  digitalWrite(TRIGGERPIN, HIGH);
  delayMicroseconds(12); 
  digitalWrite(TRIGGERPIN, LOW);
  duration = pulseIn(ECHOPIN, HIGH);
  distance = (duration/2) / 29.1;
  float tinggi = 29.4-distance;
  float volume = ((14.3*14.5*tinggi)/1000);
  if (volume < vol){
    digitalWrite(ppmb,HIGH);
    Blynk.virtualWrite(V13,255);
    digitalWrite(pump,LOW);
    Blynk.virtualWrite(V14,0);
  }
  else if (volume < 1){
    digitalWrite(ppmb,HIGH);
    Blynk.virtualWrite(V13,255);
    digitalWrite(pump,LOW);
    Blynk.virtualWrite(V14,0);
      }
else{
    digitalWrite(ppmb,LOW);
    Blynk.virtualWrite(V13,0);
    digitalWrite(pump,HIGH);
    Blynk.virtualWrite(V14,255);
}
  Blynk.virtualWrite(V5,volume);
}
BLYNK_WRITE(V6){
  phm = param.asFloat();
}

float ph(){
  lcd2.print(0,0,"Min pH is 5.5");
  lcd2.print(1,1,"Min ppm is 800");
  int phread = analogRead(phsensor);
  float phv =  5.0 / 1024.0*phread;
  pH_value = 3.5*phv+0.12;
     if (pH_value < phm){
    digitalWrite(phdown,LOW);
    Blynk.virtualWrite(V16,0);
    digitalWrite(phup,HIGH);
    Blynk.virtualWrite(V15,255);
    delay(300);
    digitalWrite(phup,LOW);
    Blynk.virtualWrite(V15,0);
    delay(3000);
  }
  else if (pH_value > 8){
    digitalWrite(phup,LOW);
    Blynk.virtualWrite(V15,0);
    digitalWrite(phdown,HIGH);
    Blynk.virtualWrite(V16,255);
    delay(300);
    digitalWrite(phdown,LOW);
    Blynk.virtualWrite(V16,0);
    delay(3000);
  }
  else{
    digitalWrite(phup,LOW);
    Blynk.virtualWrite(V15,0);
    digitalWrite(phdown,LOW);
    Blynk.virtualWrite(V16,0);
  }

  Blynk.virtualWrite(V7,pH_value);
  delay(500);
}

//tds
BLYNK_WRITE(V8){
  tdsen = param.asFloat();
}

float ppm(){
    gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
    gravityTds.update();  //sample and calculate
    tdsValue = gravityTds.getTdsValue();  // then get the value
    if(tdsValue < tdsen){
      digitalWrite(ppma,HIGH);
      Blynk.virtualWrite(V17,255);
      Blynk.virtualWrite(V18,255);
      delay(300);
      digitalWrite(ppma,LOW);
      Blynk.virtualWrite(V17,0);
      Blynk.virtualWrite(V18,0);
      delay(3000);      
    }
    Blynk.virtualWrite(V9,tdsValue);
    delay(500);
}
BLYNK_WRITE(V10){
  int luxin = param.asInt();
  digitalWrite(growl,luxin);
  digitalWrite(fan,luxin);
  }
void luxmet(){
  LDR_value=analogRead(ldr);
  lux=(250.0/(ADC_value*LDR_value))-50.0;
  
  Blynk.virtualWrite(V11,lux);
  delay(500);
}

//temp
void temprt()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  Blynk.virtualWrite(V12, t);
}


void loop(){
  Blynk.run();
  waterlevel();
  ph();
  ppm();
  timer.run();
  temprt();
  luxmet();
}
