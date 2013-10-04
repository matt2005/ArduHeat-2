#include <LiquidCrystal.h>
#include <math.h>
#include <SPI.h>
#include <Ethernet.h>
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

LiquidCrystal lcd(A3, A2, 5, 7, 3, 2);

//variables for the current temperature reading: variableIN is the TMP36 temp. sensor. for inside components safety
const int sensorPin = A1;
const int sensorINPin = A0;
const int numReadings = 20;
const int numReadingsIN = 20;
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

float desiredT = 20;// desired room temperature

//variables for the botton system to set roomT, piu is +, meno is -
const int piuPin = A4;
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

int internet;



// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {  0x90, 0xA2, 0xDA, 0x0D, 0xA8, 0xAF };
IPAddress ip(192,168,1,3);

// initialize the library instance:
EthernetClient client;

const unsigned long requestInterval = 15000;  // delay between requests

char serverName[] = "api.twitter.com";  // twitter URL

boolean requested;                   // whether you've made a request since connecting
unsigned long lastAttemptTime = 0;            // last time you connected to the server, in milliseconds

String currentLine = "";            // string to hold the text from server
String tweet = "";                  // string to hold the tweet
boolean readingTweet = false;       // if you're currently reading the tweet





void setup() {
 digitalWrite(9, HIGH);    //turn on LCD backlight
 
 pinMode(A5, OUTPUT);
 pinMode(0, OUTPUT);
 pinMode(buttonPin, INPUT);
 pinMode(LCDbacklightPin, OUTPUT);
 pinMode(piuPin, INPUT);
 pinMode(menoPin, INPUT);
 pinMode(8, OUTPUT);
 pinMode(7, OUTPUT);
 pinMode(1, INPUT);
 
 // Serial.begin(9600);//open serial communication
 
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
    
    
    
    
    // reserve space for the strings:
  currentLine.reserve(256);
  tweet.reserve(150);

  // attempt a DHCP connection:
 
 // Serial.println("Attempting to get an IP address using DHCP:");
  if (!Ethernet.begin(mac)) {
    // if DHCP fails, start with a hard-coded address:
 //   Serial.println("failed to get an IP address using DHCP, trying manually");
    Ethernet.begin(mac, ip);
  }
 // Serial.print("My address:");
 // Serial.println(Ethernet.localIP());
  // connect to Twitter:
  connectToServer();

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
  delay(5);
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
    digitalWrite(0, HIGH);
  }else if(TEMPr > desiredT){
    digitalWrite(A5, LOW);
    lcd.print("  OFF");//heating status printed at end of screen
    digitalWrite(0, LOW);
  } else if(TEMPr == desiredT){
  }
  
 /* delay(50); //delay, for calculations and printing
  Serial.print(TEMPr);
  Serial.print("  ");
  Serial.print(TEMPrIN);
  Serial.print("  ");
  Serial.println(desiredT); */
  
  //stop 10 seconds if inside temperature (TEMPrIN) increases a lot)
  if(TEMPrIN > 40){
    delay(10000);
  }else{
    delay(10);
  
  }
  
  
  

  
  
  
  
  
 // if(1 == HIGH){
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
       //   Serial.println(tweet);
          char carray[tweet.length() + 1]; //determine size of the array
          tweet.toCharArray(carray, sizeof(carray)); //put readStringinto an array
          float f = atof(carray+1); //convert the array into an Integer
          internet = digitalRead(1);
         if(internet == HIGH){desiredT = f;
                               lcd.setCursor(15,0);
                               lcd.print("i");
         }
         else if(internet == LOW){lcd.setCursor(15,0);
                                   lcd.print("l");}
      //    Serial.println(tweet);  //so you can see the captured string
       //   Serial.println(carray);
        //  Serial.println(f);
          // close the connection to the server:
          client.stop(); 
        }
      }
    }   
  }
  else if (millis() - lastAttemptTime > requestInterval) {
    // if you're not connected, and two minutes have passed since
    // your last connection, then attempt to connect again:
    connectToServer();
  }
}
int connectToServer() {
  // attempt to connect, and wait a millisecond:
 // Serial.println("connecting to server...");
  if (client.connect(serverName, 80)) {
   // Serial.println("making HTTP request...");
    // make HTTP GET request to twitter:
    client.println("GET /1/statuses/user_timeline.xml?screen_name=Arduheat&count=1 HTTP/1.1");
    client.println("HOST: api.twitter.com");
    client.println();
  }
  // note the time of this connect attempt:
  lastAttemptTime = millis();
  
}
  
  //desiredT = tweet;
//[with this system only the values of temperature and motor status are
//updated every loop(), writing is static since end uf setup()]

