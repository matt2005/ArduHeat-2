#include <math.h>
const int sensorPin = A4;
const int sensor2Pin = A0;
int reading;
int reading2;
float voltage;
float voltage2;
float temp;
float temp2;
float TEMP;
float TEMPR1;
float TEMPRf;


void setup() {

 Serial.begin(9600);//open serial communication

}

void loop() {
  reading = analogRead(sensorPin);
  reading2 = analogRead(sensor2Pin);
  voltage = (reading/1024.0) * 5;
  voltage2 = (reading2/1024.0) * 5;
  temp = (voltage *100);
  temp2 = (voltage2 - .5) *100;
  TEMP = (temp + temp2) / 2;
 // Serial.print(voltage);
 // Serial.print(" ");
  Serial.print(temp,3);
  Serial.println(); 
 // Serial.print(voltage2);
 // Serial.print(" ");
  Serial.print(temp2,3);
  Serial.println(); 
  Serial.print((temp2 - temp), 2);
  Serial.println();
  TEMPR1 = (round(TEMP*2));
  TEMPRf = TEMPR1/2;
  //float b = a/2;
 // Serial.println(TEMP,5);
  //Serial.println(TEMPR1,5);//TEMP,4);
 // Serial.println(TEMPRf,5);
 // Serial.print(" ");
  
  Serial.println();
 // Serial.println();
  
 /* temp = (voltage / 10.0);
  Serial.print("   ");
  Serial.print(temp,10);
  temp2 = temp * 10;
  temp3 = round(temp2);
  Serial.print("   ");
  Serial.println(temp3 / 10,10);*/
    
  delay(333); 
}
