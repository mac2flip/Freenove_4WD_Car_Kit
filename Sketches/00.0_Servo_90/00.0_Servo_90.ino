/**********************************************************************
  Product     : Freenove 4WD Car for UNO
  Description : Set servo to 90 degree
  Auther      : www.freenove.com
  Modification: 2019/08/05
**********************************************************************/
#include <Servo.h>      //servo library

#define PIN_SERVO  2    //define servo pin
#define PIN_LED    13   //UNO built-in LED

Servo servo;            //create servo object to control a servo
int servoOffset = 0;    // signed trim in degrees for center calibration

void setup() {
  servo.attach(PIN_SERVO);        //initialize servo
  pinMode(PIN_LED, OUTPUT);
  servo.write(90 + servoOffset);  //Start at center position
  delay(600);
}

void loop() {
  digitalWrite(PIN_LED, HIGH);
  servo.write(60 + servoOffset);
  delay(700);

  digitalWrite(PIN_LED, LOW);
  servo.write(90 + servoOffset);
  delay(700);

  digitalWrite(PIN_LED, HIGH);
  servo.write(120 + servoOffset);
  delay(700);

  digitalWrite(PIN_LED, LOW);
  servo.write(90 + servoOffset);
  delay(700);
}
