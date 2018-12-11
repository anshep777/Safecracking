#include <Servo.h>
Servo servoOne; //Handle puller servo
Servo servoTwo; //Handle pull back servo

int positionOne = 0; //servoOne's position
int positionTwo = 0; //servoTwo's position

void setup(){
  servoOne.attach(9);
  servoTwo.attach(10);
}

void loop(){
  servoOne.write(90);
  delay(500);
  servoOne.write(0);
  servoTwo.write(90);
  delay(500);
  servoTwo.write(0);
  delay(500);
}

