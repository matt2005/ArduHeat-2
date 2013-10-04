/*ISSUES BOX:

-if booted with internet on, "ing..." diplayed on display until "ON" or "OFF" needed

*/

#include <LiquidCrystal.h>
#include <math.h>
#include <SPI.h>
#include <Ethernet.h>
/* SETUP:
T*sensors: "face round side" left ground, right to power, centre to A0/1
RedLED: ground to ground through 200 OMH resistor, + to A5
LCD circuit and system at pages 116...
Voltage devider:
           from third pin on lcd:
               4.7k+220 OMH to +
               560+220 OMH to -
botton setup for pins 6, 8; A4.
relay
  motor: + to battery +, - to middle relay
  diode: black to -, silver to +(0)
  battery - to relay last
internet mode switch. One side to +, other to -, centre to 1.
*/

String currentLine = "";            // string to hold the text from server
String webTempString = "";                  // string to hold the temperature from the server



LiquidCrystal lcd(A3, A2, 5, 7, 3, 2);      //initiate lcd.

unsigned long currentTime;          //current time since system start
unsigned long protect;              //time variable to protect system from functioning while temperature is not calibrated
unsigned long ethernetMaintainTimeLapsed = 0;    //time lapsed since last time the IP address was maintained

//variables for the current temperature reading: variableIN is the TMP36 temp. sensor. for inside components safety
const int sensorPin = A1;
const int sensorINPin = A0;
const int numReadings = 150;
const int numReadingsIN = 150;
int reading[numReadings];
int readingIN[numReadingsIN];
int index = 0;
int indexIN = 0;
long total = 0;
long totalIN = 0;
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
float f;          //float f is the value on the temperature set from website

float desiredT = 20;// desired room temperature


//variables for the botton (local) system to set roomT, piu is +, meno is -
const int piuPin = A4;
const int menoPin = 6;
  int piuRead;
  int piuPrev = LOW;
  int menoRead;
  int menoPrev = LOW;
  long timeA = 0;
  long debounceA = 200;

//LCD backlight botton system variables
int buttonPin = 8;          //input for the botton status
int LCDbacklightPin = 9;    //output pin for the LCD backlight LED
int LCDstate = HIGH;        //current state for the LCD LED
int bottonread;             //current reading in the buttonPin
int previous = LOW;         //previous reading from the buttonPin
long time = 0;              //the time since the output for the LED was toggled last
long debounce = 200;        //debounce time to avoid flickering


int internet;          //internet switch status


//ethernet values
byte mac[] = {  0x90, 0xA2, 0xDA, 0x0D, 0xA8, 0xAF };
IPAddress ip(192,168,1,70);

// initialize the ethernet library instance:
EthernetClient client;

unsigned int intervallo = 60000;  // delay between requests

char serverName[] = "varano.eu.pn";  // website URL

boolean requested;                   // whether a request was made since connecting
unsigned long lastAttemptTime = 0;            // last time connected to the server, in milliseconds
boolean lastConnected = false;                    // state of the connection last time through the main loop




//SETUP/////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
 
 //pinmodes
 pinMode(A5, OUTPUT);      //A5 is the pin the LED for heatingON is attached to
 pinMode(0, OUTPUT);        //0 is the pin the relay is attached to
 pinMode(buttonPin, INPUT);
 pinMode(LCDbacklightPin, OUTPUT);
 pinMode(piuPin, INPUT);
 pinMode(menoPin, INPUT);
 pinMode(1, INPUT);          //1 is the pin the switch for internet/local mod is attached to
 
 digitalWrite(LCDbacklightPin, HIGH);    //turn on LCD backlight
 
 // Serial.begin(9600);//open serial communication, *nope, pins 0 and 1 used*
 
 //begin lcd and print hello message, then clear and print fixed runtime text
 lcd.begin(16, 2);
 lcd.print("Hello!");
 delay(1000);
 lcd.setCursor(0, 1);
 lcd.print("LOADING...");      // print hello, go to next line and print loading
 delay(1500);
 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print("ATTEMPTING");      //attempting connection displayed
 lcd.setCursor(0, 1);
 lcd.print("CONNECTION...");
 
 
 // initialize all the readings of temperature to 0:
 for (int thisReading = 0; thisReading < numReadings; thisReading++)
    reading[thisReading] = 0;
 for (int thisReadingIN = 0; thisReadingIN < numReadingsIN; thisReadingIN++)
    readingIN[thisReadingIN] = 0;
    
    
// reserve space for the webdata strings:
currentLine.reserve(256);
webTempString.reserve(150);


// attempt a DHCP connection:
   // Serial.println("Attempting to get an IP address using DHCP:");              *nope, pins 0 and 1 used*
   if (!Ethernet.begin(mac)) {
    //if DHCP fails, start with a hard-coded address:
    //Serial.println("failed to get an IP address using DHCP, trying manually");  *nope, pins 0 and 1 used*
      Ethernet.begin(mac, ip);
    }
  //Serial.print("My address:");                                                  *nope, pins 0 and 1 used*
  //Serial.println(Ethernet.localIP());                                           *nope, pins 0 and 1 used*
  lcd.clear();
  lcd.print(Ethernet.localIP());    //print acquired IP to lcd for 1 second
  delay(1000);
   connectToServer();      //connect to server:
   
   
   
 lcd.clear();                  //then clear and prepare lcd for inputs from the loop()
 lcd.print("DESIRED T:");
 lcd.setCursor(0,1);
 lcd.print("ROOM T:");
 //lcd.setCursor(15, 0);
 //lcd.print("~");
 
 f = 20;      //initiate float f (webtemp value) and give an arbitrary value: 20
}




//LOOP/////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  
  
   currentTime = millis();      //set current time and protect timer to the system runtime
   protect = millis();
  
  
  if(currentTime - ethernetMaintainTimeLapsed > 60000) {    //every 60 seconds maintain IP address
   //ethernet.maintain with return check
    byte returnedValue = Ethernet.maintain();
    switch (returnedValue) {
      case 0: //lcd.print(/*"nothing happened"*/ "0");
              break;
      case 1: lcd.clear();
              lcd.print("renew failed");
              //Ethernet.begin(mac); break;                            //RESET CODE HERE FOR V2.0 
              for(;;); break;
      case 2: //lcd.clear();
              //lcd.print("renew success");
              break;
      case 3: lcd.clear();
              lcd.print("rebind fail");
              for(;;); break;
      case 4: lcd.clear();
              lcd.print("rebind success");
              for(;;); break;
      default: lcd.clear();
               lcd.print("Unexpected returned value:");
               lcd.print(returnedValue); break;
    }
    ethernetMaintainTimeLapsed = currentTime;
  }
  
  
  if(currentTime - lastAttemptTime > intervallo) {      //connect to website every 'intervallo' seconds
    connectToServer();
    lastAttemptTime = currentTime;
  }
  
  
   /* if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
    lastConnected = client.connected();*/
  
  
  //website reading bit
  if (client.available()) {
   char c = client.read();
   // if you get a '<', clear the line: temp is beginning
   if (c == '<') {
     webTempString = "";
     currentLine = "";
     char c = client.read();    //read client again to eliminate '<' and start reading the temperature
     currentLine += c;
     c = client.read();
     currentLine += c;
     c = client.read();
     currentLine += c;
     c = client.read();
     currentLine += c;
     webTempString = currentLine;
     client.flush();
     client.stop();
   }
  }

  char carray[webTempString.length() + 1];           //determine size of the array
  webTempString.toCharArray(carray, sizeof(carray)); //put readString into an array
  f = atof(carray);                  //convert the array into an float
  
  
  internet = digitalRead(1);
  if(internet == HIGH){          //if the internet switch is set on internet
    desiredT = f;        //use webserver reading
    intervallo = 120000;        //keep website search interval to 2 min.
    lcd.setCursor(15,0);
    lcd.print("i");
  }else if(internet == LOW){        //else, don't
    intervallo = 300000;      //set website search interval to 5 min.
    lcd.setCursor(15,0);
    lcd.print("l");
  }
  
  
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
  // to ignore any noise on the circuit, toggle the +/- 0.5 and remember
  // the time
  if (piuRead == HIGH && piuPrev == LOW && millis() - timeA > debounceA) {
    desiredT += .5;
    timeA = millis();
  }
  if (menoRead == HIGH && menoPrev == LOW && millis() - timeA > debounceA) {
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
  delay(1);
  reading[index] = analogRead(sensorPin);
  delay(1);
  readingIN[indexIN] = analogRead(sensorINPin);                          //double readings to allow system to adapt to analog requests
  delay(1);
  readingIN[indexIN] = analogRead(sensorINPin);
  // add the reading to the total:
  total= total + reading[index];
  totalIN= totalIN + readingIN[indexIN];
  // advance to the next position in the array:
  index = index + 1;
  indexIN = indexIN + 1;
  // if at the end of the array... ...wrap around to the beginning:
  if (index >= numReadings)
    index = 0;
  if (indexIN >= numReadingsIN)
    indexIN = 0;
    //find running average of readings
  average = total / numReadings;
  averageIN = totalIN / numReadingsIN;
  delay(1);
  
  
 // reading = analogRead(sensorPin);
 // reading2 = analogRead(sensor2Pin);      //*read sensors --- obsolete, replaced by for average loop to reduce fluctuation*
  voltage = (average/1024.0) * 5;
  voltageIN = (averageIN/1024.0) * 5;      //find voltage
  temp = (voltage *100);
  tempIN = ((voltageIN - .5) *100)/* - 3.3*/;          //and temperature
  TEMP = (round(temp*2));                              //round to closest .5
  TEMPIN = (round(tempIN*2));
  TEMPr = TEMP/2;
  TEMPrIN = (TEMPIN/2)-2.5;                              //-2.5 to reduce error in sensor
  
  
  // printing and comparing temperatures
  lcd.setCursor(10,0);//set cursor to after the "desired temp" text
  lcd.print(desiredT,1);//lcd print roomT in right place
  if (protect > 15301){
    lcd.setCursor(7,1);//set cursor to after the "current T" text
    lcd.print(TEMPr,1);//lcd print current temperature
  }
  else if (protect >= 15000 && protect <= 15300){
    lcd.clear();
    lcd.print("DESIRED T:");
    lcd.setCursor(0,1);
    lcd.print("ROOM T:");
    lcd.setCursor(7,1);//set cursor to after the "room T" text
    lcd.print(TEMPr,1);//lcd print current temperature
  }
  else if (protect < 15000){
    lcd.setCursor(7,1);
    lcd.print("loading..");
  }


  if(protect > 15301){
    if(TEMPr - desiredT < -.5){
      digitalWrite(A5, HIGH);
      lcd.setCursor(13,1);
      lcd.print("ON ");//heating status printed at end of screen
      digitalWrite(0, HIGH);
    }else if(TEMPr > desiredT){
      digitalWrite(A5, LOW);
      lcd.setCursor(13,1);
      lcd.print("OFF");//heating status printed at end of screen
      digitalWrite(0, LOW);
    }//else { //this is to avoid continuos on-->off-->on-->    //technically pointless
  //}
  }
  
  
  //stop 60 seconds if inside temperature (TEMPrIN) increases a lot
  if(TEMPrIN > 45){
    delay(60000);
  }else{
    delay(3);
  }
  
  
}

//connectToServer()/////////////////////////////////////////////////////////////////////////////////////////////////
int connectToServer() {
  // if there's a successful connection:
  if (client.connect(serverName, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET http://varano.eu.pn/temp.txt HTTP/1.0");
  //  client.println("Host: www.arduino.cc");
  //  client.println("User-Agent: arduino-ethernet");
  //  client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastAttemptTime = millis();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}

//[with this system only the values of temperature and motor status are
//updated every loop(), writing is static since end of setup()]

