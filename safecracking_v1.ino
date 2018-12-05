#include <EEPROM.h>

/*
 * The user inputs their command into the serial monitor and tells the motor where to turn
 */
int encA = 2;
int encB = 3;
int motor = 5;
volatile int count = 0; 
int dir = 6; // direction indicator (forward/CW = HIGH reverse/CCW = LOW)
int dialPos;
int homePos = 0; // Zero point is assumed to be zero unless the user changes it
int realHomePos;
int realPos;
int gate;
int mspeed = 254; // motor speed (set to 254 in case no EEPROM value is selected)
int mspeed2 = mspeed; //to remember custom motor speeds
int atHome = 2;

int userSettings(int op){
 
  if (op == 1){ // store critical values in EEPROM in case of shut down
    EEPROM.put(6, count);
    EEPROM.put(8, mspeed);
    EEPROM.put(10, dialPos);
  }
  if (op == 2) {
    EEPROM.get(6, count);
    EEPROM.get(8, mspeed);
    EEPROM.get(10, dialPos);
  }
  if (op == 3) {
    EEPROM.update(6, count);
    EEPROM.update(8, mspeed);
    EEPROM.update(10, dialPos);
  }
}

void setup() {
  while(!Serial);
  Serial.begin(9600);
  pinMode(encA, INPUT);
  pinMode(encB, INPUT);
  pinMode(motor, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(gate, INPUT);
  attachInterrupt(digitalPinToInterrupt(encA), intCountA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), intCountB, CHANGE); 
  gate = digitalRead(11);
  if (gate) {
    Serial.println("Make sure to set the home position!");
  }
  userSettings(1); // change this to (2) if the robot ever dun goofs
}

void loop() {
  userMenu();
  delay(100);
  mspeed = mspeed2;
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
  while (!((count > (realPos - 84)) && (count < (realPos + 84)))) {
    if (count < realPos){ // if a higher than encoder value input, turn CW
      CWMotor();
    }
    else if (count > realPos){ // if a lower than encoder value input turn CCW
      CCWMotor();
    }
    slowDown();
  }
  stopMotor();
  userSettings(3);
}

void slowDown(){
  Serial.print("Count: ");
  Serial.println(count);
  Serial.print("Going to: ");
  Serial.println(realPos);
  Serial.print("Motor Speed: ");
  Serial.println(mspeed);
  if((count > (realPos - 2100)) && (count < (realPos + 2100))){
    mspeed = 50;
    Serial.println("slowing");
  }
  analogWrite(motor, mspeed);
}

void goHome(){ // uses the photogate to go to set zero point
  Serial.print("home position should be: ");
  Serial.println(homePos);
  if (atHome != 2){
    dialPos = homePos;
    realPos = dialPos * 84;
    goToPosition();
  }
  else Serial.println("Already home!");
  atHome = 2; // prevents repeated use of goHome function
}

void checkGate(){
  gate = digitalRead(11);
}

void setHome(){
  atHome = 3; // in case user calls goHome() was called before setHome()
  checkGate(); //reads photogate, equals 1 if clear & 0 if interupted
  mspeed = 50;
  CWMotor();
  analogWrite(motor, mspeed);
  while(gate){ // when gate = 0 it is obstructed
    checkGate();
  }
  stopMotor();
  Serial.println("Enter dial position that corresponds to the position of the flag");
  while (!Serial.available()) delay(100);
  homePos = Serial.parseInt();
  Serial.print("Home position set to ");
  Serial.println(homePos);
  dialPos = homePos;
  realPos = dialPos * 84;  
  realHomePos = homePos * 84;
  count = realPos;
  goToPosition();
}

void crackMe(){
    Serial.println("Starting safecracking algorithm!");
}

void CWMotor(){
    digitalWrite(dir, HIGH);
}

void CCWMotor(){
    digitalWrite(dir, LOW);
}

void stopMotor(){
    analogWrite(motor, 0);
}

void userMenu(){
  
  Serial.println("Input Dial Position: d ");
  Serial.println("Stop Motor: x ");
  Serial.println("Go Home: h ");
  Serial.println("Set Home Location: s ");
  Serial.println("Select Motor Speed Value: m");
  Serial.println(" ");
  while (!Serial.available()) delay(100);
    if(Serial.available()){
      char incoming = Serial.read();
      Serial.print("You Pressed: ");
      Serial.write(incoming);
      Serial.println();
      if(incoming == 'd'){ // put in a dial position
        Serial.println("Type the dial position to go to");
        while(Serial.available() == false); // wait for user input for dial position
        dialPos = Serial.parseInt();
        Serial.print("Dial position: ");
        Serial.println(dialPos);
        realPos = dialPos * 84;
        realPos = 8400 + 2*realHomePos - realPos;
        goToPosition();
        atHome = 3;
      }
      else if(incoming == 'h'){ // go to home location
        Serial.println("Going Home");
        goHome();
      }
      else if(incoming == 's'){ // set the home location
        Serial.println("Setting Home Location");
        setHome();
      }
      else if(incoming == 'm'){ // select motor speed from the EEPROM
        while(Serial.available() == false); // wait for user input
        mspeed = Serial.parseInt();
        Serial.print("Motor speed set to: ");
        Serial.println(mspeed);
        mspeed2 = mspeed;
      }
     Serial.println(" ");
  }
}
