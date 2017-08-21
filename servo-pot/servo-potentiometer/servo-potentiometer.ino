#include <Servo.h>
Servo servo;

void setup() {
	Serial.begin(9600);
	servo.attach(8);
}

void loop() {

	int potentiometerValue = analogRead(A8);
	int servoDegrees = map(potentiometerValue, 0, 1023, 10, 170);

	Serial.print(servoDegrees);
	Serial.println(" degrees");
	servo.write(servoDegrees);
	delay(500);
}