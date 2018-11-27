#include <EEPROM.h>

/*
 * The user inputs their command into the serial monitor and tells the motor where to turn
 */
int address = 8; //default mspeed address (254)
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
int homePos = 0; // Zero point is assumed to be zero unless the user changes it
int realPos;
int gate;
int gatestate;
int mspeed = 254; // motor speed (set to 254 in case no EEPROM value is selected)
int mspeed2 = mspeed; //to remember custom motor speeds
int atHome = 2;

int userSettings(int op, int store = -1){ // op = operation
  
  if(op == 1 && store == -1){ // if only one argument put in enter the motor speed values
      // enters motor speed values into the EEPROM
    int initMotorSpeed = 50; // starts at 50, each entry increases the motor speed by 51/255; making 4 values to choose from
    for(int loc = 0; loc < 9; loc += 2){ // two bytes of memory allocated to each int
      EEPROM.put(loc, initMotorSpeed);
      initMotorSpeed += 51;
    }
  }

  if(op == 2){
    // flag position for the dial
    EEPROM.put(10, store); // the user's input is stored at address 10 
    return(store); // returns what value was just stored
  }
}

void setup() {
  while(!Serial);
  Serial.begin(9600);
  pinMode(button, INPUT);
  pinMode(encA, INPUT);
  pinMode(encB, INPUT);
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(gate, INPUT);
  attachInterrupt(digitalPinToInterrupt(encA), intCountA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), intCountB, CHANGE); 
  userSettings(1);
}

void loop() {
  gate = digitalRead(11);
  userMenu();
  delay(100);
  EEPROM.update(address, mspeed);
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
  if ((count > (realPos - 70)) && (count < (realPos + 70))) {
    slowDown();
  }
  else if(realPos > count){ // if a higher than encoder value input, turn CW
    CWMotor();
    digitalWrite(6, HIGH);
    while(realPos > count){
      slowDown();
    }
  }
  else if(realPos < count){ // if a lower than encoder value input turn CCW
    CCWMotor();
    digitalWrite(6, LOW);
    while(realPos < count){
      slowDown();
    }
  }
}

void slowDown(){
  Serial.print("Count: ");
  Serial.println(count);
  Serial.print("Going to: ");
  Serial.println(realPos);
  Serial.print("Motor Speed: ");
  Serial.println(mspeed);
//  Serial.print("Motor Speed 2: ");
//  Serial.println(mspeed2);
  if((count > (realPos - 70)) && (count < (realPos + 70))){
    stopMotor();
    Serial.println("stopping");
  }
  else if((count > (realPos - 2500)) && (count < (realPos + 2500))){
    EEPROM.update(0, mspeed);
    Serial.println("slowing");
  }
}

void goHome(){ // uses the photogate to go to set zero point
  EEPROM.get(10, homePos); // fetches the stored user input for the home location
  Serial.print("home position should be: ");
  Serial.println(homePos);
  if (atHome != 2){
    dialPos = homePos;
    realPos = dialPos * 84;
    goToPosition();
  }
  else { Serial.println("Already home!"); }
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
    homePos = Serial.parseInt(); // homePos is a correction factor to be applied to the goToPosition function
    userSettings(2, homePos); // updates the stored EEPROM value at address 10
    Serial.print("Zero point set to: ");
    Serial.println(homePos);
  }
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
        goToPosition();
        atHome = 3;
      }
      else if(incoming == 'h'){ // go to home location
        Serial.println("Going Home");
        goHome();
        atHome = 2;
      }
      else if(incoming == 's'){ // set the home location
        Serial.println("Setting Home Location");
        while(Serial.available() == false); // wait for user input for dial position
        homePos = Serial.parseInt();
        setHome();
      }
      else if(incoming == 'm'){ // select motor speed from the EEPROM
        Serial.println("Enter EEPROM address to select motor speed: ");
        Serial.println("EEPROM Address Values");
        Serial.println("Address 0 = 50");
        Serial.println("Address 2 = 101");
        Serial.println("Address 4 = 152");
        Serial.println("Address 6 = 203");
        Serial.println("Address 8 = 254");
        while(Serial.available() == false); // wait for user input
        address = Serial.parseInt();
        if(address %2 == 0 && address < 9){
          Serial.print("Motor speed set to: ");
          EEPROM.get(address, mspeed);
          Serial.println(mspeed);
        }
        else{
          Serial.println("Incorrect input, even numbers 0 - 8 only");
        }
        mspeed2 = mspeed;
      }
     Serial.println(" ");
  }
}
