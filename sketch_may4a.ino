//old_bootloader

#define temperature_value 38
//#define temperature_value 37.75
#define overHeatingDeviation 0.4

#define CLK 2
#define DIO 3
#define TM1637_pin_VCC 4
#define delay_time 1000
#define change_time 5000

#include "GyverTM1637.h"
#include <OneWire.h> 
#include <DallasTemperature.h>
#include "DHT.h"

GyverTM1637 disp(CLK, DIO);

uint32_t Now, clocktimer;
boolean flag;

#define DHTTYPE DHT11

#define ONE_WIRE_BUS 7 //ds18b20 data pin
#define DHTPIN 6 //DHT 11 data pin 
#define speakerPort 5 //port for speaker or tweaker
 
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);
DHT dht(DHTPIN, DHTTYPE);

float temperature;
float Temperature;
float humidity;

//вивід
bool mode;
byte temp_1, temp_2, temp_3, temp_4;
int temp;
int humdt;
unsigned long last_time;
unsigned long last_change;
unsigned long last_sound;
bool flag_for_sound = true;

//нагрівний елемент 
#define rele_pin_digit 10
#define reserve_rele_pin_digit 11
#define rele_pin_VCC 9
bool flag_rele = 0;
bool isEmergency = 0;

void setup() {
  Serial.begin(9600); 
  
  pinMode(TM1637_pin_VCC, OUTPUT);
  digitalWrite(TM1637_pin_VCC, HIGH);

  pinMode(rele_pin_VCC, OUTPUT);
  digitalWrite(rele_pin_VCC, HIGH);

  pinMode(rele_pin_digit, OUTPUT);
  pinMode(reserve_rele_pin_digit, OUTPUT);
  pinMode(speakerPort, OUTPUT);

  sensors.begin(); 
  dht.begin();
  disp.brightness(5); 
  disp.clear();
}

void turnOnSpeaker(){
  if (millis() - last_sound > 300){
    last_sound = millis();
    digitalWrite(speakerPort, flag_for_sound);
    flag_for_sound = !flag_for_sound;
  }
}
void turnOffSpeaker(){
  digitalWrite(speakerPort, 0);
}

void displayData(){
  temperature = sensors.getTempCByIndex(0);
  humidity = dht.readHumidity();
  // temperature = 35;
  // humidity = 50;
  if(!mode){
    temp = temperature*100;
    if(millis()-last_time > delay_time){
      last_time = millis();
      temp_1 = (temp/1)%10;
      temp_2 = (temp/10)%10;
      temp_3 = (temp/100)%10;
      temp_4 = (temp/1000)%10;
      byte data[4] = {temp_4, temp_3, temp_2, temp_1};
      //disp.point(1);
      disp.display(data);
    }
  }
  if(mode){
    humdt = humidity*100;
    if(millis()-last_time > delay_time){
      last_time = millis();
      temp_1 = (humdt/1)%10;
      temp_2 = (humdt/10)%10;
      temp_3 = (humdt/100)%10;
      temp_4 = (humdt/1000)%10;
      disp.display(0, temp_4); 
      disp.display(1, temp_3); 
      disp.displayByte(2, _empty);
      disp.displayByte(3, _H);
    }
  }
  if(millis()-last_change>change_time){
    last_change = millis();
    mode = !mode;
  }
}

void loop() {
  sensors.requestTemperatures();
  float h = dht.readHumidity();
  Temperature = sensors.getTempCByIndex(0);

  if(Temperature<temperature_value && !flag_rele){
    Serial.println("rele, 1");
    disp.point(1);
    digitalWrite(rele_pin_digit,1);
  }
  if (Temperature>=temperature_value && !flag_rele){
    Serial.println("rele, 0");
    disp.point(0);
    digitalWrite(rele_pin_digit,0);
    flag_rele = 1;                     
  }
  if (Temperature<=temperature_value-0.25 && flag_rele){
    Serial.println("rele, 1");
    disp.point(1);
    digitalWrite(A1,1);
    flag_rele = 0;
  } 

  //EMERGENCY  
  if (Temperature>temperature_value+overHeatingDeviation){
    turnOnSpeaker();
    digitalWrite(reserve_rele_pin_digit, 1);
    digitalWrite(rele_pin_digit,0);
    isEmergency = 1;
    Serial.println("Overheating... Reserved rele was activated. Speaker activated.");
  } else if (Temperature<=temperature_value-0.25 && isEmergency){
    turnOffSpeaker();
    digitalWrite(reserve_rele_pin_digit, 0);
    digitalWrite(rele_pin_digit, 1);
    isEmergency = 0;
    Serial.println("Normal state... Speaker deactivated.");
  } else if(Temperature<temperature_value-2) {
    turnOnSpeaker();
    Serial.println("Overcooling...");
  } else if (Temperature>=temperature_value-0.25 && Temperature<temperature_value+overHeatingDeviation){
    turnOffSpeaker();
  } else {
    turnOffSpeaker();
  }

  Serial.print("temperature: ");
  Serial.println(sensors.getTempCByIndex(0));
  Serial.println(Temperature);
  Serial.print("Humidity: ");
  Serial.println(h);
  displayData();
}
