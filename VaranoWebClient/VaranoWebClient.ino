/*
  Repeating Web client
 
 This sketch connects to a a web server and makes a request
 using a Wiznet Ethernet shield. You can use the Arduino Ethernet shield, or
 the Adafruit Ethernet shield, either one will work, as long as it's got
 a Wiznet Ethernet module on board.
 
 This example uses DNS, by assigning the Ethernet client with a MAC address,
 IP address, and DNS address.
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 19 Apr 2012
 by Tom Igoe
 
 http://arduino.cc/en/Tutorial/WebClientRepeating
 This code is in the public domain.
 
 */

#include <SPI.h>
#include <Ethernet.h>

// assign a MAC address for the ethernet controller.
// fill in your address here:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
// fill in an available IP address on your network here,
// for manual configuration:
IPAddress ip(192,168,1,70);

// fill in your Domain Name Server address here:
//IPAddress myDns("varano.eu.pn");

// initialize the library instance:
EthernetClient client;

char server[] = "varano.eu.pn";

unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
boolean lastConnected = false;                    // state of the connection last time through the main loop
const unsigned long postingInterval = 6000;  // delay between updates, in milliseconds
						  // the "L" is needed to use long type numbers
String currentLine = "";            // string to hold the text from server
String tweet = "";                  // string to hold the tweet



void setup() {
  // start serial port:
  Serial.begin(9600);
  // give the ethernet module time to boot up:
  delay(1000);
  // start the Ethernet connection using a fixed IP address and DNS server:
  Ethernet.begin(mac);
  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  
    currentLine.reserve(256);
    char c;
}

void loop() {
  //delay(1);
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
   // currentLine += c;
    // if you get a newline, clear the line:
   /*   if (c == '<') {
        tweet = "";
        currentLine = "";
        char c = client.read();
        Serial.print("ciao: ");
        Serial.println(c);
        currentLine += c;
        c = client.read();
        currentLine += c;
        c = client.read();
        currentLine += c;
        c = client.read();
        currentLine += c;
        tweet = currentLine;
        }
      }
  Serial.print(tweet);*/
       // currentLine = "";
       // currentLine += c;
     // }
   // Serial.print(c);
 // }
}
  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    httpRequest();
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET http://varano.eu.pn/temp.txt HTTP/1.0");
  //  client.println("Host: www.arduino.cc");
  //  client.println("User-Agent: arduino-ethernet");
  //  client.println("Connection: close");
  //  client.println("Accept: text/plain, text/html, charset=iso-8859-1"); /* */
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}




