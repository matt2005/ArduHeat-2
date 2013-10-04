//potentiometer: middle to A1, sides to + and -
//transistor: middle to motor- and diodeBlackSide
//            left of shiny back side to ground
//            right to pin 9
//motor: + to battery +, - to middle transistor
//diode: black to middle transistor, silver to battery +
//connect grounds
const int motorPin = 9;
const int potPin = A1;
void setup() {
  Serial.begin(9600);
  pinMode(motorPin, OUTPUT);
  pinMode(potPin, INPUT);
}

void loop() {
 int motorSpeed = analogRead(potPin)/4;
 int motorPower = (motorSpeed/2.55);
 analogWrite(motorPin, motorSpeed);
 Serial.print("Motor input(0-255): ");
 Serial.print(motorSpeed);
 Serial.print(".   Power: ");
 Serial.print(motorPower);
 Serial.println("%");
delay(350);
}
