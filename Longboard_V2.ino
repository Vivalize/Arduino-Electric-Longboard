#include <Wire.h>
#include <Servo.h>
#include "Nunchuk.h"

//Personal Constants
int maxAcceleration = 5;
int maxDeceleration = 10;
double throttleCap = .4;
double zThrottle = .15;
double cSpeedModifier = 2.0;

//Hardware Constants
int minIn = 3;
int maxIn = 127;
int minOut = 1000;
int maxOut = 2000;
int timeOut = 1000;

//Other variables
int currentSpeed;
int targetSpeed;
int newSpeed;
int numberOfDuplicates = 0;
unsigned long lastValidReadTime = millis();
boolean inTimeout = true;
int last_xJoy = 0, last_yJoy = 0, last_xAcc = 0, last_yAcc = 0, last_zAcc = 0, last_zBut = 0, last_cBut = 0;
Servo motor;

void setup() {
    Serial.begin(9600);
    currentSpeed = minOut;
    targetSpeed = minOut;
    newSpeed = minOut;
    Wire.begin();
    nunchuk_init();
    motor.attach(9);
    Serial.println("Setup Complete");
}

void loop() {

    //Nunchuk is successfully read
    if (nunchuk_read() &&
        filterExtremes(nunchuk_accelX(), nunchuk_accelY(), nunchuk_accelZ(), nunchuk_joystickX(), nunchuk_joystickY()) &&
        (!inTimeout || !isDuplicate())) {

        if (inTimeout) Serial.println("Nunchuk Connected");
        inTimeout = false;
        if (!isDuplicate()) lastValidReadTime = millis();
        
        targetSpeed = getTargetSpeed();
        newSpeed = getNewSpeed(currentSpeed, targetSpeed);
        printSpeedStats();

        motor.write(newSpeed);
        currentSpeed = newSpeed;
      }
      
      //If there's an error reading the nunchuk (such as from
      //disconnection), make sure to time out if needed
      else {
        if (millis() - lastValidReadTime > timeOut) {
          motor.writeMicroseconds(minOut);
          if (!inTimeout) Serial.println("Nunchuk Disconnected");
          inTimeout = true;
        }
      }

    delay(20);
}



//Considers all controller inputs and returns a speed
//that the board should accelerate/decelerate to
int getTargetSpeed() {
  int throttle = 0;

  //If using Z
  if (nunchuk_buttonZ()) throttle = ((maxOut-minOut)*zThrottle)+minOut;

  //Otherwise use joystick reading
  else throttle = map(nunchuk_joystickY(), minIn, maxIn, minOut, ((maxOut-minOut)*throttleCap)+minOut );

  //Apply 2x modifier if pressing C
  if (nunchuk_buttonC()) throttle = ((throttle-minOut)*cSpeedModifier)+minOut;

  //Ensure signal is within bounds
  if (throttle < minOut) throttle = minOut;
  if (throttle > maxOut) throttle = maxOut;
  
  return throttle;
}



//Considers the currentSpeed and targetSpeed to determine what
//speed we should use next to gradually approach the target speed
int getNewSpeed(int currentSpeed, int targetSpeed) {
  int speedDifference = targetSpeed - currentSpeed;
  
  //If close to target, set speed to target
  if (speedDifference > (-1 * maxAcceleration) && speedDifference < maxAcceleration) return targetSpeed;

  //If accelerating...
  else if (speedDifference > 0) return currentSpeed + maxAcceleration;

  //If decelerating...
  else return currentSpeed - maxDeceleration;
}



//Returns true if the input signal is corrupted
boolean filterExtremes(int xAcc, int yAcc, int zAcc, int xJoy, int yJoy) {
  if (!(
    ((xAcc > 505 && xAcc < 515) || (xAcc > -515 && xAcc < -505)) &&
    ((yAcc > 505 && yAcc < 515) || (yAcc > -515 && yAcc < -505)) &&
    ((zAcc > 505 && zAcc < 515) || (zAcc > -515 && zAcc < -505)) &&
    ((xJoy > 125 && xJoy < 130) || (xJoy > -130 && xJoy < -125)) &&
    ((yJoy > 125 && yJoy < 130) || (yJoy > -130 && yJoy < -125)) )) return true;
  else return false;
}




//Checks whether the current reading is a duplicate of the last
bool isDuplicate() {
  if (
    nunchuk_joystickX() == last_xJoy &&
    nunchuk_joystickY() == last_yJoy &&
    nunchuk_accelX() == last_xAcc &&
    nunchuk_accelY() == last_yAcc &&
    nunchuk_accelZ() == last_zAcc &&
    nunchuk_buttonZ() == last_zBut &&
    nunchuk_buttonC() == last_cBut ) {
      return true;
    }
    else {
      last_xJoy = nunchuk_joystickX();
      last_yJoy = nunchuk_joystickY();
      last_xAcc = nunchuk_accelX();
      last_yAcc = nunchuk_accelY();
      last_zAcc = nunchuk_accelZ();
      last_zBut = nunchuk_buttonZ();
      last_cBut = nunchuk_buttonC();
      return false;
    }
}





//DEBUG FUNCTIONS

void printSpeedStats() {
    Serial.print("Joystick: ");
    Serial.print(nunchuk_joystickY());
    Serial.print("\tSpeed: ");
    Serial.print(currentSpeed);
    Serial.print("/");
    Serial.print(targetSpeed);
    Serial.println("");
}

void printVars(int xJoy, int yJoy, int xAcc, int yAcc, int zAcc, int zBut, int cBut) {
    Serial.print("Joysticks: ");
    Serial.print(xJoy);
    Serial.print(", ");
    Serial.print(yJoy);
    Serial.print(", Acceleration: ");
    Serial.print(xAcc);
    Serial.print(", ");
    Serial.print(yAcc);
    Serial.print(", ");
    Serial.print(zAcc);
    Serial.print(", Buttons: ");
    Serial.print(zBut);
    Serial.print(", ");
    Serial.print(cBut);
    Serial.println("");
}

void printNunchukStats() {
  nunchuk_print();
}

