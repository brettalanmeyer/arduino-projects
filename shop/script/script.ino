#include <AccelStepper.h>
#include <Keypad.h>
#include <LiquidCrystal.h>



// MotorInterfaceType
const int EASY_DRIVER_INTERFACE = 1;

// number of gates to configure
const int GATE_COUNT = 8;

// stepper motor steps per single revolution
const int STEPS_PER_REVOLUTION = 1600;

// number of revolutions to open/close blast gate, around 4" of travel
const int REVOLUTIONS_PER_CYCLE = 12.8;

// stepper intialization values
const int STEPPER_MAX_SPEED = 5000;
const int STEPPER_ACCELERATION = 5000;

// initializing stepper motors
AccelStepper stepperJointer		(EASY_DRIVER_INTERFACE,	52,	50);
AccelStepper stepperDrillPress	(EASY_DRIVER_INTERFACE,	44,	42);
AccelStepper stepperSanders		(EASY_DRIVER_INTERFACE,	36,	34);
AccelStepper stepperPlaner		(EASY_DRIVER_INTERFACE,	28,	26);
AccelStepper stepperRouterTable	(EASY_DRIVER_INTERFACE,	53,	51);
AccelStepper stepperBandSaw		(EASY_DRIVER_INTERFACE,	45,	43);
AccelStepper stepperTableSaw	(EASY_DRIVER_INTERFACE,	37,	35);

struct Gate {
	String name;
	int toolNumber;
	int togglePin;
	boolean stepperEnabled;
	AccelStepper stepper;
	boolean open;
	int sleepPin;
};

Gate gates[GATE_COUNT] = {
	{ "Jointer",		1,		46,		true,	stepperJointer,			false,	48		},
	{ "Drill Press",	2,		38,		true,	stepperDrillPress,		false,	40		},
	{ "Sanders",		3,		30,		true,	stepperSanders,			false,	32		},
	{ "Planer", 		4,		22,		true,	stepperPlaner,			false,	24		},
	{ "Router Table",	5,		47,		true,	stepperRouterTable,		false,	49		},
	{ "Band Saw",		6,		39,		true,	stepperBandSaw,			false,	41		},
	{ "Miter Saw",		7,		2,		false,	NULL,					false,	NULL	},
	{ "Table Tab",		8,		31,		true,	stepperTableSaw,		false,	33		},
};



// 4 x 4 keypad module
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
	{ '1', '2', '3', 'A' },
	{ '4', '5', '6', 'B' },
	{ '7', '8', '9', 'C' },
	{ '*', '0', '#', 'D' }
};

// connect to the row pinouts of the keypad starting with left most pin 8, 7, 6, 5
byte rowPins[ROWS] = { 29, 27, 25, 23 };

// connect to the column pinouts of the keypad ending with right most pin 4, 3, 2, 1
byte colPins[COLS] = { 3, 4, 5, 6 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// sections of menu
enum MenuSelectionState {
	MenuSelectionStateDefault,
	MenuSelectionStatePageOne,
	MenuSelectionStatePageTwo,
	MenuSelectionStatePageThree,
};
MenuSelectionState menuSelectionState = MenuSelectionStateDefault;

// actions selectable from menu
enum ActionType {
	ActionTypeOpenAllGates,
	ActionTypeCloseAllGates,
	ActionTypeOpenGateNumber,
	ActionTypeCloseGateNumber,
	ActionTypeOpenGateIncrementally,
	ActionTypeCloseGateIncrementally
};
ActionType actionType = NULL;

// value of gate selected from keypad, index of gate will be value - 1
int selectedGateNumber = NULL;



// toggles power to lcd backlight via transistor
const int LCD_TRANSISTOR_PIN = 13;

// delay in seconds for lcd backlight to remain on
const int LCD_BACKLIGHT_DELAY = 10;

boolean lcdBackLightOn = false;
int lcdTurnOffDelay = millis() / 1000;

// initialize the library
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);



void initializeLcdPanel() {
	Serial.println("Initializing LCD panel...");

	pinMode(LCD_TRANSISTOR_PIN, OUTPUT);

	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);

	printDefaultMessage();
}

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

// TODO
void initializeInfraredLed() {
	Serial.println("Initializing infrared LED...");
}

// TODO
void toggleDustCollector() {
	Serial.println("Transmitting dust collector signal");
}

void turnLcdBackLightOn() {
	Serial.println("Turning on LCD BackLight...");

	lcd.display();
	digitalWrite(LCD_TRANSISTOR_PIN, HIGH);
	lcdBackLightOn = true;
}

void turnLcdBackLightOff() {
	Serial.println("Turning off LCD BackLight...");

	lcd.noDisplay();
	digitalWrite(LCD_TRANSISTOR_PIN, LOW);
	lcdBackLightOn = false;
}

void printDefaultMessage() {
	printToLcd("Brett's Shop", "# for options");
}

void printToLcd(String line1, String line2) {
	lcd.clear();

	lcd.setCursor(0, 0);
	lcd.print(line1);

	lcd.setCursor(0, 1);
	lcd.print(line2);

	turnLcdBackLightOn();
	lcdTurnOffDelay = millis() / 1000;
}

void updateLcd() {
	// turn off LCD backlight after given period of time
	if(lcdBackLightOn && ((millis() / 1000) - lcdTurnOffDelay > LCD_BACKLIGHT_DELAY)){
		turnLcdBackLightOff();
	}
}

void checkKeyPad() {

	char key = keypad.getKey();

	if (key) {

		if (isControlKey(key)) {

			if (key == '*') {
				cancelMenu();

			} else if(key == '#') {
				cycleMenu();
			}

		} else if(isNumberKey(key)) {

			if (actionType == NULL) {
				selectInitialOptions(key);

			} else if (actionType == ActionTypeOpenGateNumber) {
				selectedGateNumber = (int) key;
				openGate(selectedGateNumber - 1);
				selectedGateNumber = NULL;

			} else if (actionType == ActionTypeCloseGateNumber) {
				selectedGateNumber = (int) key;
				closeGate(selectedGateNumber - 1);
				selectedGateNumber = NULL;

			} else if (actionType == ActionTypeOpenGateIncrementally) {
				printToLcd("Select a value", "to open by...");
				if (selectedGateNumber == NULL) {
					selectedGateNumber = (int) key;
				} else {
					openGateIncrementally(selectedGateNumber - 1, (int) key);
				}

			} else if (actionType == ActionTypeCloseGateIncrementally) {
				printToLcd("Select a value", "to close by...");
				if (selectedGateNumber == NULL) {
					selectedGateNumber = (int) key;
				} else {
					closeGateIncrementally(selectedGateNumber - 1, (int) key);
				}
			}
		}
	}
}

void cancelMenu() {
	menuSelectionState = MenuSelectionStateDefault;
	actionType = NULL;
	selectedGateNumber = NULL;
	printDefaultMessage();
}

void cycleMenu() {
	if (menuSelectionState == MenuSelectionStateDefault) {
		menuSelectionState == MenuSelectionStatePageOne;
		printToLcd("1: Open Gates", "2: Close Gates");

	} else if (menuSelectionState == MenuSelectionStatePageOne) {
		menuSelectionState == MenuSelectionStatePageTwo;
		printToLcd("3: Open Gate#", "4: Close Gate#");

	} else if (menuSelectionState == MenuSelectionStatePageTwo) {
		menuSelectionState == MenuSelectionStatePageThree;
		printToLcd("5: OpenX Gate#", "6: CloseX Gate#");

	} else if (menuSelectionState == MenuSelectionStatePageThree) {
		menuSelectionState == MenuSelectionStateDefault;
		printDefaultMessage();
	}
}

void selectInitialOptions(char key) {
	if (key == '1') {
		actionType = ActionTypeOpenAllGates;
		printToLcd("Opening All", "Gates...");
		openAllGates();
		printToLcd("Gates Opened", "");

	} else if (key == '2') {
		actionType = ActionTypeCloseAllGates;
		printToLcd("Closing All", "Gates...");
		closeAllGates(NULL);
		printToLcd("Gates Closed", "");

	} else if (key == '3') {
		actionType = ActionTypeOpenGateNumber;
		printToLcd("Select Gate", "Number");

	} else if (key == '4') {
		actionType = ActionTypeCloseGateNumber;
		printToLcd("Select Gate", "Number");

	} else if (key == '5') {
		actionType = ActionTypeOpenGateIncrementally;
		printToLcd("Select Gate", "Number");

	} else if (key == '6') {
		actionType = ActionTypeCloseGateIncrementally;
		printToLcd("Select Gate", "Number");
	}
}

boolean isLetterKey(char c) {
	return (c == 'A' || c == 'B' || c == 'C' || c == 'D');
}

boolean isControlKey(char c) {
	return (c == '*' || c == '#');
}

boolean isNumberKey(char c) {
	return (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9');
}

void checkToggleButton() {
	for (int i = 0; i < GATE_COUNT; i++) {

		if (digitalRead(gates[i].togglePin) == LOW) {

			toggleDustCollector();

			if (!gates[i].open) {
				closeAllGates(i);
				openGate(i);
			}

			break;
		}

	}
}

void openAllGates() {
	Serial.println("Closing all gates");

	for(int i = 0; i < GATE_COUNT; i++){
		if(!gates[i].open){
			openGate(i);
		}
	}
}

void openGate(int index) {
	Serial.println("Opening tool " + index);

	printToLcd("Opening Gate " + String(index) + " for the " + gates[index].name, "");
	gates[index].open = true;

	long newPosition = gates[index].stepper.currentPosition() + (STEPS_PER_REVOLUTION * REVOLUTIONS_PER_CYCLE);
	moveGate(index, newPosition);
}

void openGateIncrementally(int index, int value) {
	int revolutions = getRevolutions(value);
	Serial.println("Opening tool " + String(index) + " amount: " + String(revolutions));
	long newPosition = gates[index].stepper.currentPosition() + (STEPS_PER_REVOLUTION * revolutions);
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

void closeGateIncrementally(int index, int value) {
	int revolutions = getRevolutions(value);
	Serial.println("Opening tool " + String(index) + " amount: " + String(revolutions));
	long newPosition = gates[index].stepper.currentPosition() - (STEPS_PER_REVOLUTION * revolutions);
	moveGate(index, newPosition);
}

void moveGate(int index, long newPosition) {
	wakeTool(index);
	gates[index].stepper.runToNewPosition(newPosition);
	sleepTool(index);
}

float getRevolutions(int value) {
	// numbers will be 10ths of a revolution except 0 which will be treated as 10 thus 1 full revolution
	if (value == 0) {
		return 1.0;
	}

	return (float) value / 10.0;
}

void wakeTool(int index) {
	Serial.println("Waking " + gates[index].name + " gate");
	digitalWrite(gates[index].sleepPin, HIGH);
}

void sleepTool(int index) {
	Serial.println("Sleeping " + gates[index].name + " gate");
	digitalWrite(gates[index].sleepPin, LOW);
}



// Main Program Methods
void setup() {
	Serial.begin(9600);
	initializeLcdPanel();
	initializeToggleButtons();
	initializeStepperMotors();
	initializeInfraredLed();
}

void loop() {
	checkKeyPad();
	checkToggleButton();
	updateLcd();
}
