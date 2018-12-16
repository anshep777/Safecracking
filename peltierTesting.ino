// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// this code is public domain, enjoy!

#include <AFMotor.h>


AF_DCMotor motor2(2);
AF_DCMotor motor3(3);


void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Motor test!");
}

void loop() {

  motor2.setSpeed(150);
  motor3.setSpeed(150);

  
 
  motor2.run(FORWARD); 
  motor3.run(FORWARD);  
  delay(10000);


  motor2.setSpeed(150);
  motor3.setSpeed(150);

  motor2.run(BACKWARD); 
  motor3.run(BACKWARD); 
  
  delay(10000);

}
