#include <AccelStepper.h>

// AccelStepper stepper(1, 13, 12);
AccelStepper stepper(1, 52, 50);

const int STEPS_PER_REVOLUTION = 1600;

const int SLEEP_PIN = 48;
// LOW is sleep, HIGH is enabled

// 1 = Easy Driver interface
// NANO Pin 2 connected to STEP pin of Easy Driver
// NANO Pin 3 connected to DIR pin of Easy Driver

void setup(){
	// Change these to suit your stepper if you want
	Serial.begin(9600);
	stepper.setMaxSpeed(5000);
	stepper.setAcceleration(5000);

	Serial.println("Max Speed");
	Serial.println(stepper.maxSpeed());

	// setup sleeper
	pinMode(SLEEP_PIN, OUTPUT);
}

void loop(){

	Serial.println("SLEEP");
	digitalWrite(SLEEP_PIN, LOW);

	Serial.println("OPEN");
	stepper.runToNewPosition(STEPS_PER_REVOLUTION * 12.8);
	delay(5000);

	Serial.println("CLOSE");
	stepper.runToNewPosition(0);
	delay(5000);


	Serial.println("WAKE");
	digitalWrite(SLEEP_PIN, HIGH);

	Serial.println("OPEN");
	stepper.runToNewPosition(STEPS_PER_REVOLUTION * 12.8);
	delay(5000);

	Serial.println("CLOSE");
	stepper.runToNewPosition(0);
	delay(5000);

}


// http://www.airspayce.com/mikem/arduino/AccelStepper/annotated.html
// https://brainy-bits.com/blogs/tutorials/setting-stepper-motors-home-position-using-accelstepper
// https://brainy-bits.com