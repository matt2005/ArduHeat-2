/* switch
 * 
 * Each time the input pin goes from LOW to HIGH (e.g. because of a push-button
 * press), the output pin is toggled from LOW to HIGH or HIGH to LOW.  There's
 * a minimum delay between toggles to debounce the circuit (i.e. to ignore
 * noise).  
 *
 * David A. Mellis
 * 21 November 2006
 */
int cacca = 0;
int muPin = 6;         // the number of the input pin
int ballzPin = 13;
//int outPin = A5;       // the number of the output pin

//int state = HIGH;
//int state2 = HIGH;// the current state of the output pin
int muRead;           // the current reading from the input pin
int muPrev = LOW;    // the previous reading from the input pin
int ballzRead;
int ballzPrev = LOW;

// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers

void setup()
{
  Serial.begin(9600);
  pinMode(muPin, INPUT);
  pinMode(ballzPin, INPUT);
  //pinMode(outPin, OUTPUT);
  
}

void loop()
{
  muRead = digitalRead(muPin);
  ballzRead = digitalRead(ballzPin);

  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember
  // the time
  if (muRead == HIGH && muPrev == LOW /*&& millis() - time > debounce*/) {
    cacca -= 1;
    time = millis();
  }
  if (ballzRead == HIGH && ballzPrev == LOW /*&& millis() - time > debounce*/) {
    cacca += 1;
    time = millis();
  }

  muPrev = muRead;
  ballzPrev = ballzRead;
  Serial.println(cacca);
}
