/*
 * The user inputs their command into the serial monitor and tells the motor where to turn
 */
#include <EEPROM.h>

int encA = 2;
int encB = 3;
int motor1 = 9;
int motor2 = 10;
int timer = 0;
int button = 7;
int debounce = 50; // debounce time
int state = LOW; // current state of output
bool prev = LOW;
int reading; // current state of the pushbutton
volatile int count = 0; 
int dir = 6; // direction indicator (forward/CW = HIGH reverse/CCW = LOW)
int turnTo;
int dialPos;
int homePos;
int gate;
int gatestate;
int mspeed = 255; // motor speed (set to 255 in case no EEPROM value is selected)
int eepMotor = 0; // Initial EEPROM address variable for the motor speed

void setup() {
  while(!Serial);
  Serial.begin(9600);
  Serial.println("Input Dial Position: d ");
  Serial.println("Stop Motor: x ");
  Serial.println("Go Home: h ");
  Serial.println("Set Home Location: s ");
  Serial.println("Select Motor Speed Value: m");
  Serial.println(" ");
  pinMode(button, INPUT);
  pinMode(encA, INPUT);
  pinMode(encB, INPUT);
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(gate, INPUT);
  attachInterrupt(digitalPinToInterrupt(encA), intCountA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), intCountB, CHANGE); 

  // enters motor speed values into the EEPROM
  for(int i = 0; i <= 5; i++){ // address
    for(float ee = 20; ee <= 255; ee = ee + 51){ // each entry increases the motor speed by 51/255
      EEPROM.put(i, ee);
      mspeed = ee;
    }
  }
}

void loop() {
  gate = digitalRead(11);
  if(Serial.available()){
    int incoming = Serial.read();
    Serial.print("You Pressed: ");
    Serial.write(incoming);
    Serial.println();

    if(incoming == 'd'){ // put in a dial position
      Serial.println("Type the dial position to go to");
      while(Serial.available() == false); // wait for user input for dial position
      dialPos = Serial.parseInt();

      Serial.print("Dial position: ");
      Serial.println(dialPos);
    }
    else if(incoming == 'h'){ // go to home location
      Serial.println("Going Home");
      goHome;
    }
    else if(incoming == 's'){ // set the home location
      Serial.println("Setting Home Location");
//      while(Serial.available() == false); // wait for user input for dial position
//      homePos = Serial.parseInt();
      setHome();
    }
    else if(incoming == 'm'){ // select motor speed from the EEPROM
      Serial.println("Enter EEPROM address to select motor speed: ");
      while(Serial.available() == false); // wait for user input
      int incoming = Serial.read();
      Serial.print("Motor speed set to: ");
      EEPROM.get(eepMotor, mspeed);
      Serial.println(mspeed);
//        Serial.println(EEPROM.read(eepMotor));
//        mspeed = EEPROM.read(eepMotor);
    }
    goToPosition();
  }
}

void intCountA(){ 
  // add 1 to count for clockwise
  // subtract 1 from count for counterclockwise
  // Check for low to high on encoder A
  if(digitalRead(encA) == HIGH){
    // check B to see which way the encoder is turning
    if(digitalRead(encB) == HIGH){ // if encoder A and encoder B are HIGH, add one to count
      count++; //CW
    }
    else{
      count--; //CCW // if encoder A is HIGH and encoder B is LOW, decrement count
    }
  }
  // check for high to low on encoder A
  else{
    // check encoder B to compare to A
    if(digitalRead(encB) == LOW){ // if encoder B is LOW and encoder A is LOW, add one to count
      count++; //CW
    }
    else{
      count--; //CCW // if encoder B is HIGH subtract one from count
    }
  }
}

void intCountB(){
  // add 1 to count for clockwise
  // subtract 1 from count for counterclockwise
  // Check for low to high on encoder B
  if(digitalRead(encB) == HIGH){
    // check B to see which way the encoder is turning
    if(digitalRead(encA) == HIGH){
      count--; //CW
    }
    else{
      count++; //CCW
    }
  }
  // check for high to low on encoder B
  else{
    // check encoder A to compare to B
    if(digitalRead(encA) == LOW){
      count--; //CW
    }
    else{
      count++; //CCW
    }
  }
}

void goToPosition(){ // walks the motor to a position then stops it
  if((dialPos * 84) > count){ // if a higher than encoder value input, turn CW
    CWMotor();
    digitalWrite(6, HIGH);
//    while(dialPos * 84 > count){
//      Serial.print("dial position * 84: ");
//      Serial.println(dialPos * 84);
//      Serial.print("count: ");
//      Serial.println(count);
//      slowDown();
//    }
    stopMotor();
  }
  if((dialPos * 84) < count){ // if a lower than encoder value input turn CCW
    CCWMotor();
    digitalWrite(6, LOW);
//    while(dialPos * 84 < count){
//      Serial.print("dial position * 84: ");
//      Serial.println(dialPos * 84);
//      Serial.print("count: ");
//      Serial.println(count);
//      slowDown();
//    }
    stopMotor();
  }
}

void goHome(){ // uses the photogate to go to set zero point
  if(homePos * 84 == count){
    CWMotor();
  }
  if(!gate) stopMotor();
  Serial.print("home position should be: ");
  Serial.println(homePos);
}

void setHome(){
  Serial.print("Line up the flag with the sensor");
  if(!gate){
    Serial.print("Flag lined up, enter zero point: ");
    while(Serial.available() == false){ // wait for user input for dial position
      if(homePos == 'x'){ // press x to exit set home menu
        break;
      }
    }
    homePos = Serial.parseInt();
    Serial.print("Zero point set to: ");
    Serial.println(homePos);
  }
  dialPos = homePos; // for the goToPosition function
  goToPosition();
}

void slowDown(){
  // slow down by 70 if within 30 of the target dial value
  if((dialPos * 84) < ((count - 30)|(count + 30))){ // if dial position is less than encoder value
    mspeed = mspeed - 70;
  }
  if((dialPos * 84) > ((count - 30)|(count + 30))){ // if dial position is greater than encoder value
    mspeed = mspeed - 70;
  }
  //slow down by 140 if within 20 of target dial value
  if((dialPos * 84) < ((count - 20)|(count + 20))){ // if dial position is less than encoder value
    mspeed = mspeed - 70;
  }
  if((dialPos * 84) > ((count - 20)|(count + 20))){ // if dial position is greater than encoder value
    mspeed = mspeed - 70;
  }
    //slow down by 235 if within 7 of target dial value
  if((dialPos * 84) < ((count - 7)|(count + 7))){ // if dial position is less than encoder value
    mspeed = mspeed - 95;
  }
  if((dialPos * 84) > ((count - 7)|(count + 7))){ // if dial position is greater than encoder value
    mspeed = mspeed - 95;
  }
  // final speed should be 15/255 coming either CW or CCW
}

void CCWMotor(){
    analogWrite(motor1, mspeed); // sets one side of the H-bridge high and the other low
    digitalWrite(motor2, LOW);
}

void CWMotor(){
    analogWrite(motor2, mspeed);
    digitalWrite(motor1, LOW);
}

void stopMotor(){
    analogWrite(motor2, 0);
    digitalWrite(motor1, LOW);
}

