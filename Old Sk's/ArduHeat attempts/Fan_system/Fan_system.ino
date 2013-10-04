#include <LiquidCrystal.h>
#include <math.h>

/* SETUP:
T*sensors: "face round side" left ground, right to power, centre to A0
RedLED: ground to ground through 200 OMH resistor, + to 7
GreenLED: ground to ground through 200 OMH resistor, + to 8
LCD circuit and system at pages 116...
Voltage devider:
           from third pin on lcd:
               4.7k+220 OMH to +
               560+220 OMH to -
botton setup for pins 6, 13, (next).
*NO  motor: + to battery +, - to middle transistor
**NO  diode: black to middle transistor, silver to battery +
**NO  transistor: middle to motor- and diodeBlackSide
**NO        left of shiny back side to ground
**NO        right to pin 9
**NO  connect grounds  */

LiquidCrystal lcd(12, 11, 5, 7, 3, 2);

//variables for the current temperature reading: variableIN is the TMP36 temp. sensor. for inside components safety
const int sensorPin = A1;
const int sensorINPin = A0;
const int numReadings = 10;
const int numReadingsIN = 10;
int reading[numReadings];
int readingIN[numReadingsIN];
int index = 0;
int indexIN = 0;
int total = 0;
int totalIN = 0;
int average = 0;
int averageIN = 0;
float voltage;
float voltageIN;            //reading (0-1023), voltage and temperature on each sensor
float temp;
float tempIN;
float TEMP;              //temperatures and variables to round off later
float TEMPIN;
float TEMPr;
float TEMPrIN;

float desiredT;// desired room temperature

//variables for the botton system to set roomT, piu is +, meno is -
const int piuPin = 13;
const int menoPin = 6;
  int piuRead;
  int piuPrev = LOW;
  int menoRead;
  int menoPrev = LOW;
  long timeA = 0;
  long debounceA = 200;

//LCD backlight botton system variables
int buttonPin = 8;//input for the botton status
int LCDbacklightPin = 9;//output for the LCD backlight LED
int LCDstate = HIGH;//current state for the LCD LED
int bottonread;//current reading in the buttonPin
int previous = LOW;//previous reading from the buttonPin
long time = 0;//the time since the output for the LED was toggled last
long debounce = 500;//debounce time to avoid flickering





void setup() {
 digitalWrite(9, HIGH);    //turn on LCD backlight
 
 pinMode(A5, OUTPUT);
 pinMode(buttonPin, INPUT);
 pinMode(LCDbacklightPin, OUTPUT);
 pinMode(piuPin, INPUT);
 pinMode(menoPin, INPUT);
 pinMode(8, OUTPUT);
 pinMode(7, OUTPUT);
 
  Serial.begin(9600);//open serial communication
 
 lcd.begin(16, 2);
 lcd.print("Hello!");
 lcd.setCursor(0, 1);
 lcd.print("LOADING...");// print hello, go to next line and print loading 
 delay(2000);//keep the loading screen for 3 seconds
 lcd.clear();//then clear and prepare lcd for inputs from the loop()
 lcd.print("DESIRED T:");
 lcd.setCursor(0,1);
 lcd.print("ROOM T:");
 
 // initialize all the readings to 0:
 for (int thisReading = 0; thisReading < numReadings; thisReading++)
    reading[thisReading] = 0;
 for (int thisReadingIN = 0; thisReadingIN < numReadingsIN; thisReadingIN++)
    readingIN[thisReadingIN] = 0;
    
 desiredT = 20;
}







void loop() {
  //LCD backlight switch code
  bottonread = digitalRead(buttonPin);
  // if the input on botton just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember the time
  if (bottonread == HIGH && previous == LOW && millis() - time > debounce) {
    if (LCDstate == HIGH)
      LCDstate = LOW;
    else
      LCDstate = HIGH;
    time = millis();    
  }
  digitalWrite(LCDbacklightPin, LCDstate);
  previous = bottonread;
  
  
  //roomT setting bottons system code
  piuRead = digitalRead(piuPin);
  menoRead = digitalRead(menoPin);
  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember
  // the time
  if (piuRead == HIGH && piuPrev == LOW && millis() - time > debounceA) {
    desiredT += .5;
    timeA = millis();
  }
  if (menoRead == HIGH && menoPrev == LOW && millis() - time > debounceA) {
    desiredT -= .5;
    timeA = millis();
  }
  piuPrev = piuRead;
  menoPrev = menoRead;
  
  
  //temperature measurement code
  // subtract the last reading:
  total= total - reading[index];
  totalIN= totalIN - readingIN[indexIN];
  // read from the sensor:
  reading[index] = analogRead(sensorPin);
  delay(50);
  readingIN[indexIN] = analogRead(sensorINPin);
  // add the reading to the total:
  total= total + reading[index];
  totalIN= totalIN + readingIN[indexIN];
  // advance to the next position in the array:
  index = index + 1;
  indexIN = indexIN + 1;
  // if we're at the end of the array... ...wrap around to the beginning:
  if (index >= numReadings)
    index = 0;
  if (indexIN >= numReadingsIN)
    indexIN = 0;
    //find running average of readings
  average = total / numReadings;
  averageIN = totalIN / numReadingsIN;
  delay(1);
  
  
 // reading = analogRead(sensorPin);
 // reading2 = analogRead(sensor2Pin);      //read sensors --- obsolete, replaced by for thingy loop to reduce fluctuation
  voltage = (average/1024.0) * 5;
  voltageIN = (averageIN/1024.0) * 5;      //find voltage
  temp = (voltage *100);
  tempIN = ((voltageIN - .5) *100)/* - 3.3*/;          //and temperature
  TEMP = (round(temp*2));               //and find average
  TEMPIN = (round(tempIN*2));
  TEMPr = TEMP/2;                    //round to closest .5
  TEMPrIN = TEMPIN/2;
  
  
  
  
  // printing and comparing temperatures
  lcd.setCursor(10,0);//set cursor to after the "desired temp" text
  lcd.print(desiredT,1);//lcd print roomT in right place
  lcd.setCursor(7,1);//set cursor to after the "current T" text
  lcd.print(TEMPr,1);//lcd print current temperature

  if(TEMPr < desiredT){
    digitalWrite(A5, HIGH);
    lcd.print("   ON  ");//heating status printed at end of screen
  }else if(TEMPr > desiredT){
    digitalWrite(A5, LOW);
    lcd.print("  OFF");//heating status printed at end of screen
  } else if(TEMPr == desiredT){
  }
  
  delay(50); //delay, for calculations and printing
  Serial.print(TEMPr);
  Serial.print("  ");
  Serial.print(TEMPrIN);
  Serial.print("  ");
  Serial.println(desiredT);
  
  //stop 10 seconds if inside temperature (TEMPrIN) increases a lot)
  if(TEMPrIN > 40){
    delay(10000);
  }else{
  }
//[with this system only the values of temperature and motor status are
//updated every loop(), writing is static since end uf setup()]
}
