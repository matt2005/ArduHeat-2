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




LiquidCrystal lcd(A3, A2, 5, 7, 3, 2);

unsigned long reconnect;
unsigned long protect;

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
float f;          //float f is the value on the tweet

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

// initialize the library instance:
EthernetClient client;

const unsigned int requestInterval = 5000;  // delay between requests

char serverName[] = "api.twitter.com";  // twitter URL

boolean requested;                   // whether a request was made since connecting
unsigned long lastAttemptTime = 0;            // last time connected to the server, in milliseconds

String currentLine = "";            // string to hold the text from server
String tweet = "";                 // string to hold the tweet
boolean readingTweet = false;       // if currently reading the tweet




//SETUP/////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
 
 //pinmodes
 pinMode(A5, OUTPUT);      //A5 is the pin the LED for heatingON is attached to
 pinMode(0, OUTPUT);        //0 is the pin the realy is attached to
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
 lcd.print("ATTEMPTING");
 lcd.setCursor(0, 1);
 lcd.print("CONNECTION...");
 
 
 // initialize all the readings of temperature to 0:
 for (int thisReading = 0; thisReading < numReadings; thisReading++)
    reading[thisReading] = 0;
 for (int thisReadingIN = 0; thisReadingIN < numReadingsIN; thisReadingIN++)
    readingIN[thisReadingIN] = 0;
    
    
// reserve space for the twitter strings:
currentLine.reserve(256);
tweet.reserve(150);


// attempt a DHCP connection:
   // Serial.println("Attempting to get an IP address using DHCP:");              *nope, pins 0 and 1 used*
   if (!Ethernet.begin(mac)) {
    //if DHCP fails, start with a hard-coded address:
    //Serial.println("failed to get an IP address using DHCP, trying manually");  *nope, pins 0 and 1 used*
     /* lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("connection");
      lcd.setCursor(0, 1);
      lcd.print("failed");
      delay(1000);
      lcd.setCursor(0, 0);
      lcd.print("RETRYING...");
      lcd.setCursor(0, 1);
      lcd.print("please wait");*/
      Ethernet.begin(mac, ip);
    }
  //Serial.print("My address:");                                                  *nope, pins 0 and 1 used*
  //Serial.println(Ethernet.localIP());                                           *nope, pins 0 and 1 used*
  //connect to Twitter:
  lcd.clear();
  lcd.print(Ethernet.localIP());
  delay(1000);
   connectToServer();
   
   
   
 lcd.clear();                  //then clear and prepare lcd for inputs from the loop()
 lcd.print("DESIRED T:");
 lcd.setCursor(0,1);
 lcd.print("ROOM T:");
 lcd.setCursor(15, 0);
 lcd.print("~");
 
 f = 30;      //initiate float f (tweet value) and give an arbitrary value: 21
}




//LOOP/////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  
   reconnect = millis();
   protect = millis();
  
if (reconnect > 600000 && reconnect - lastAttemptTime > 15000){
  connectToServer();
  reconnect = millis();
}
  
  //twitter reading bit
  //if(1 == HIGH){ //if internet switch is on internet setting, then go on twitter for reading *done later*
    if (client.connected()) {
    if (client.available()) {
      // read incoming bytes:
      char inChar = client.read();

      // add incoming byte to end of line:
      currentLine += inChar; 

      // if you get a newline, clear the line:
      if (inChar == '\n') {
        currentLine = "";
      } 
      // if the current line ends with <text>, it will
      // be followed by the tweet:
      if ( currentLine.endsWith("<text>")) {
        // tweet is beginning. Clear the tweet string:
        readingTweet = true; 
        tweet = "";
      }
      // if you're currently reading the bytes of a tweet,
      // add them to the tweet String:
      if (readingTweet) {
        if (inChar != '<') {
          tweet += inChar;
        } 
        else {
          // if you got a "<" character,
          // you've reached the end of the tweet:
          readingTweet = false;
       // Serial.println(tweet);       *nope, pins 0 and 1 used*
          char carray[tweet.length() + 1];           //determine size of the array
          tweet.toCharArray(carray, sizeof(carray)); //put readString into an array
          f = atof(carray+1);                  //convert the array into an float
//        internet = digitalRead(1);
//         if(internet == HIGH){
//           desiredT = f;
//           lcd.setCursor(15,0);
//           lcd.print("i");
//         }
//         else if(internet == LOW){
//           lcd.setCursor(15,0);
//           lcd.print("l");
//         }
          // Serial.println(tweet);  //so you can see the captured string *nope, pins 0 and 1 used*
          
          // close the connection to the server:
          client.stop();
        }
      }
    }   
  }
  else if (millis() - lastAttemptTime > requestInterval) {
    // if you're not connected, and 15 seconds have passed since
    // your last connection, then attempt to connect again:
    connectToServer();
    //reconnect = millis();
  }
  
  
  internet = digitalRead(1);
  if(internet == HIGH){
           desiredT = f;
           lcd.setCursor(15,0);
           lcd.print("i");
         }
         else if(internet == LOW){
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
  delay(3);
  reading[index] = analogRead(sensorPin);
  delay(5);
  readingIN[indexIN] = analogRead(sensorINPin);
  delay(3);
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
  TEMPrIN = (TEMPIN/2)-2.5;                              //-7 to reduce error in sensor
  
  
  // printing and comparing temperatures
  lcd.setCursor(10,0);//set cursor to after the "desired temp" text
  lcd.print(desiredT,1);//lcd print roomT in right place
if(protect > 15000){
  lcd.setCursor(7,1);//set cursor to after the "current T" text
  lcd.print(TEMPr,1);//lcd print current temperature
}
else{
  lcd.setCursor(7,1);
  lcd.print("loading..");
}

if(protect > 15000){
  if(TEMPr < desiredT){
    digitalWrite(A5, HIGH);
    lcd.print("   ON  ");//heating status printed at end of screen
    digitalWrite(0, HIGH);
  }else if(TEMPr > desiredT){
    digitalWrite(A5, LOW);
    lcd.print("  OFF");//heating status printed at end of screen
    digitalWrite(0, LOW);
  } else if(TEMPr == desiredT){ //this is to avoid continuos on-->off-->on-->
  }
}
  
  //stop 10 seconds if inside temperature (TEMPrIN) increases a lot
  if(TEMPrIN > 45){
    delay(30000);
  }else{
    delay(5);
  }
}


//connectToServer()/////////////////////////////////////////////////////////////////////////////////////////////////
int connectToServer() {
 // attempt to connect, and wait a millisecond:
 // Serial.println("connecting to server...");    *nope, pins 0 and 1 used*
  if (client.connect(serverName, 80)) {
   // Serial.println("making HTTP request...");   *nope, pins 0 and 1 used*
    // make HTTP GET request to twitter:
    client.println("GET /1/statuses/user_timeline.xml?screen_name=Arduheat&count=1 HTTP/1.1");
    client.println("HOST: api.twitter.com");
    client.println();
  }
  // note the time of this connect attempt:
  lastAttemptTime = millis();
  
}

//[with this system only the values of temperature and motor status are
//updated every loop(), writing is static since end of setup()]

