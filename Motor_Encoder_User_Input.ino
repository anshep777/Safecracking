/*
 * The user inputs their command into the serial monitor and tells the motor where to turn
 */


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

void setup() {
  Serial.begin(9600);
  while(Serial == false);
  Serial.println("Input Dial Position: d ");
  Serial.println("Stop Motor: x ");
  Serial.println("Go Home: h ");
  Serial.println("Set Home Location: s ");
  pinMode(button, INPUT);
  pinMode(encA, INPUT);
  pinMode(encB, INPUT);
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);
  pinMode(dir, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(encA), intCountA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), intCountB, CHANGE); 
}

void loop() {
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
    }
    else if(incoming == 'x'){ // stop the motor
      stopMotor();
      Serial.println("motor stopped");
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
    while(dialPos * 84 > count){
      Serial.print("dial position * 84: ");
      Serial.println(dialPos * 84);
      Serial.print("count: ");
      Serial.println(count);
    }
    stopMotor();
  }
  if((dialPos * 84) < count){ // if a lower than encoder value input turn CCW
    CCWMotor();
    digitalWrite(6, LOW);
    while(dialPos * 84 < count){
      Serial.print("dial position * 84: ");
      Serial.println(dialPos * 84);
      Serial.print("count: ");
      Serial.println(count);
    }
    stopMotor();
  }
}

int dialCount(){ // converts the user input into a a dial location for the encoder
  int dialLocation = dialPos * 84; 
  return dialLocation;
  Serial.print(dialLocation);
}

void goHome(){
  
}

void CCWMotor(){
    analogWrite(motor1, 255);
    digitalWrite(motor2, LOW);
}

void CWMotor(){
    analogWrite(motor2, 255);
    digitalWrite(motor1, LOW);
}

void stopMotor(){
    analogWrite(motor2, 0);
    digitalWrite(motor1, LOW);
}

