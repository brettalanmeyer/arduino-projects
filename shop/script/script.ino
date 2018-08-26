#include <AccelStepper.h>

// number of gates to configure
const int GATE_COUNT = 8;

// toggles dust collector TODO
const int DUST_COLLECTOR_PIN = 0000;

// MotorInterfaceType
const int EASY_DRIVER_INTERFACE = 1;

// stepper motor steps per single revolution
const int STEPS_PER_REVOLUTION = 1600;

// number of revolutions to open/close blast gate, around 4" of travel
const int REVOLUTIONS_PER_CYCLE = 12.8;

// stepper intialization values
const int STEPPER_MAX_SPEED = 5000;
const int STEPPER_ACCELERATION = 5000;

// declare stepper motors, interface type, step pin, direction pin
AccelStepper stepper1 (EASY_DRIVER_INTERFACE,	52,	50);
AccelStepper stepper2 (EASY_DRIVER_INTERFACE,	44,	42);
AccelStepper stepper3 (EASY_DRIVER_INTERFACE,	36,	34);
AccelStepper stepper4 (EASY_DRIVER_INTERFACE,	28,	26);
AccelStepper stepper5 (EASY_DRIVER_INTERFACE,	53,	51);
AccelStepper stepper6 (EASY_DRIVER_INTERFACE,	45,	43);
AccelStepper stepper7 (EASY_DRIVER_INTERFACE,	37,	35);

// pins and stepper motors for toggling, waking, and sleeping motors
struct Gate {
	// pin for toggle buttons to open/close gate
	int togglePin;
	// determines if stepper is used at this gate
	boolean stepperEnabled;
	// stepper object
	AccelStepper stepper;
	// is stepper currently open
	boolean open;
	// pin to control sleep/wake
	int sleepPin;
};

// define pins and steppers for each gate
Gate gates[GATE_COUNT] = {
	{ 46,	true,	stepper1,	false,	48		},
	{ 38,	true,	stepper2,	false,	40		},
	{ 30,	true,	stepper3,	false,	32		},
	{ 22,	true,	stepper4,	false,	24		},
	{ 47,	true,	stepper5,	false,	49		},
	{ 39,	true,	stepper6,	false,	41		},
	{ 31,	true,	stepper7,	false,	33		},
	{ 2,	false,	NULL,		false,	NULL	},
};

void initializeToggleButtons() {
	Serial.println("Initializing toggle buttons...");

	for(int i = 0; i < GATE_COUNT; i++){
		pinMode(gates[i].togglePin, INPUT_PULLUP);
	}
}

void initializeStepperMotors() {
	Serial.println("Initializing stepper motors...");

	for (int i = 0; i < GATE_COUNT; i++) {
		if (gates[i].stepperEnabled) {
			gates[i].stepper.setMaxSpeed(STEPPER_MAX_SPEED);
			gates[i].stepper.setAcceleration(STEPPER_ACCELERATION);
			pinMode(gates[i].sleepPin, OUTPUT);
		}
	}
}

void initializeDustCollector() {
	Serial.println("Initializing dust collector...");
	pinMode(DUST_COLLECTOR_PIN, OUTPUT);
}

void toggleDustCollector() {
	Serial.println("Toggling dust collector remote");
	digitalWrite(DUST_COLLECTOR_PIN, HIGH);
    delay(250);
    digitalWrite(DUST_COLLECTOR_PIN, LOW);
}

void checkToggleButton() {
	for (int i = 0; i < GATE_COUNT; i++) {
		if (digitalRead(gates[i].togglePin) == LOW) {
			toggleDustCollector();

			if (!gates[i].open) {
				openGate(i);
				closeAllGates(i);
			}
			break;
		}
	}
}

void openGate(int index) {
	Serial.println("Opening tool " + index);

	gates[index].open = true;

	long newPosition = gates[index].stepper.currentPosition() + (STEPS_PER_REVOLUTION * REVOLUTIONS_PER_CYCLE);
	moveGate(index, newPosition);
}

void closeAllGates(int index) {
	Serial.println("Closing all gates");

	for(int i = 0; i < GATE_COUNT; i++){
		if(index != i && gates[i].open){
			closeGate(i);
		}
	}
}

void closeGate(int index) {
	Serial.println("Closing tool " + index);
	gates[index].open = false;
	long newPosition = gates[index].stepper.currentPosition() - (STEPS_PER_REVOLUTION * REVOLUTIONS_PER_CYCLE);
	moveGate(index, newPosition);
}

void moveGate(int index, long newPosition) {
	wakeTool(index);
	gates[index].stepper.runToNewPosition(newPosition);
	sleepTool(index);
}

void wakeTool(int index) {
	Serial.println("Waking gate " + index);
	digitalWrite(gates[index].sleepPin, HIGH);
}

void sleepTool(int index) {
	Serial.println("Sleeping gate " + index);
	digitalWrite(gates[index].sleepPin, LOW);
}

// Main Program Setup
void setup() {
	Serial.begin(9600);
	initializeToggleButtons();
	initializeStepperMotors();
	initializeDustCollector();
}

// Main Program Loop
void loop() {
	checkToggleButton();
}
