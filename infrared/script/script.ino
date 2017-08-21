// https://learn.sparkfun.com/tutorials/ir-communication


// https://learn.adafruit.com/using-an-infrared-library/hardware-needed
#include "IRLibAll.h"

//Create a receiver object to listen on pin 2
IRrecvPCI myReceiver(2);

//Create a decoder object
IRdecode myDecoder;

void setup() {
	Serial.begin(9600);
	delay(2000); while (!Serial); //delay for Leonardo
	myReceiver.enableIRIn(); // Start the receiver
	Serial.println(F("Ready to receive IR signals"));
}

void loop() {
	//Continue looping until you get a complete signal received
	if (myReceiver.getResults()) {
		myDecoder.decode();
		myDecoder.dumpResults(true);
		myReceiver.enableIRIn();
	}
}
