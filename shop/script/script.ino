#include <AccelStepper.h>
#include <Keypad.h>
#include <LiquidCrystal.h>



// MotorInterfaceType
const int EASY_DRIVER_INTERFACE = 1;

// number of tools to configure
const int TOOL_COUNT = 8;

// stepper motor steps per single revolution
const int STEPS_PER_REVOLUTION = 1600;

// number of revolutions to open/close blast gate, around 4" of travel
const int REVOLUTIONS_PER_CYCLE = 12.8;

// initializing stepper motors
AccelStepper stepperJointer		(EASY_DRIVER_INTERFACE,	52,	50);
AccelStepper stepperDrillPress	(EASY_DRIVER_INTERFACE,	44,	42);
AccelStepper stepperSanders		(EASY_DRIVER_INTERFACE,	36,	34);
AccelStepper stepperPlaner		(EASY_DRIVER_INTERFACE,	28,	26);
AccelStepper stepperRouterTable	(EASY_DRIVER_INTERFACE,	53,	51);
AccelStepper stepperBandSaw		(EASY_DRIVER_INTERFACE,	45,	43);
AccelStepper stepperTableSaw	(EASY_DRIVER_INTERFACE,	37,	35);

struct Tool {
	String name;
	int toolNumber;
	int togglePin;
	boolean stepperEnabled;
	AccelStepper stepper;
	boolean open;
	int sleepPin;
};

Tool tools[TOOL_COUNT] = {
	{ "Jointer",		1,		46,		true,	stepperJointer,			false,	48		},
	{ "Drill Press",	2,		38,		true,	stepperDrillPress,		false,	40		},
	{ "Sanders",		3,		30,		true,	stepperSanders,			false,	32		},
	{ "Planer", 		4,		22,		true,	stepperPlaner,			false,	24		},
	{ "Router Table",	5,		47,		true,	stepperRouterTable,		false,	49		},
	{ "Band Saw",		6,		39,		true,	stepperBandSaw,			false,	41		},
	{ "Miter Saw",		7,		2,		false,	NULL,					false,	NULL	},
	{ "Table Tab",		8,		31,		true,	stepperTableSaw,		false,	33		},
};



// keypad is a 4x4 pad with 0-9, A-D, *, and #
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

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

enum MenuSelectionState {
	MenuSelectionStateDefault,
	MenuSelectionStatePageOne,
	MenuSelectionStatePageTwo,
	MenuSelectionStatePageThree,
};

enum ToolCommandType {
	ToolCommandTypeOpenAllGates,
	ToolCommandTypeCloseAllGates,
	ToolCommandTypeOpenGateNumber,
	ToolCommandTypeCloseGateNumber,
	ToolCommandTypeOpenGateIncrementally,
	ToolCommandTypeCloseGateIncrementally
};

MenuSelectionState menuSelectionState = MenuSelectionStateDefault;
ToolCommandType toolCommandType = NULL;
int selectedToolNumber = NULL;



// toggles power to lcd backlight
const int LCD_TRANSISTOR_PIN = 13;

// delay in seconds for lcd backlight to remain on
const int LCD_BACKLIGHT_DELAY = 10;

boolean lcdBackLightOn = false;
int lcdTurnOffDelay = millis() / 1000;

// initialize the library
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);



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
}

void initializeLcdPanel() {
	Serial.println("Initializing LCD panel...");

	pinMode(LCD_TRANSISTOR_PIN, OUTPUT);

	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);

	printDefaultMessage();
}

void initializeToggleButtons() {
	Serial.println("Initializing toggle buttons...");

	for(int i = 0; i < TOOL_COUNT; i++){
		pinMode(tools[i].togglePin, INPUT_PULLUP);
	}
}

void initializeStepperMotors() {
	for (int i = 0; i < TOOL_COUNT; i++) {

		if (tools[i].stepperEnabled) {
			tools[i].stepper.setMaxSpeed(5000);
			tools[i].stepper.setAcceleration(5000);

			pinMode(tools[i].sleepPin, OUTPUT);
		}

	}
}

void initializeInfraredLed() {
	Serial.println("Initializing infrared led");
}

void toggleDustCollector() {
	Serial.println("Transmitting dust collector signal");
}

void turnLcdBackLightOn() {
	lcd.display();
	digitalWrite(LCD_TRANSISTOR_PIN, HIGH);
	lcdBackLightOn = true;
}

void turnLcdBackLightOff() {
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
				cycleMenus();
			}

		} else if(isNumberKey(key)) {

			if (toolCommandType == NULL) {
				selectInitialOptions(key);

			} else if (toolCommandType == ToolCommandTypeOpenGateNumber) {
				selectedToolNumber = (int) key;
				openGate(selectedToolNumber - 1);
				selectedToolNumber = NULL;

			} else if (toolCommandType == ToolCommandTypeCloseGateNumber) {
				selectedToolNumber = (int) key;
				closeGate(selectedToolNumber - 1);
				selectedToolNumber = NULL;

			} else if (toolCommandType == ToolCommandTypeOpenGateIncrementally) {
				printToLcd("Select a value", "to open by...");
				if (selectedToolNumber == NULL) {
					selectedToolNumber = (int) key;
				} else {
					openGateIncrementally(selectedToolNumber - 1, (int) key);
				}

			} else if (toolCommandType == ToolCommandTypeCloseGateIncrementally) {
				printToLcd("Select a value", "to close by...");
				if (selectedToolNumber == NULL) {
					selectedToolNumber = (int) key;
				} else {
					closeGateIncrementally(selectedToolNumber - 1, (int) key);
				}
			}
		}
	}
}

void cancelMenu() {
	menuSelectionState = MenuSelectionStateDefault;
	toolCommandType = NULL;
	selectedToolNumber = NULL;
	printDefaultMessage();
}

void cycleMenus() {
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
		toolCommandType = ToolCommandTypeOpenAllGates;
		printToLcd("Opening All", "Gates...");
		openAllGates();
		printToLcd("Gates Opened", "");

	} else if (key == '2') {
		toolCommandType = ToolCommandTypeCloseAllGates;
		printToLcd("Closing All", "Gates...");
		closeAllGates(NULL);
		printToLcd("Gates Closed", "");

	} else if (key == '3') {
		toolCommandType = ToolCommandTypeOpenGateNumber;
		printToLcd("Select Gate", "Number");

	} else if (key == '4') {
		toolCommandType = ToolCommandTypeCloseGateNumber;
		printToLcd("Select Gate", "Number");

	} else if (key == '5') {
		toolCommandType = ToolCommandTypeOpenGateIncrementally;
		printToLcd("Select Gate", "Number");

	} else if (key == '6') {
		toolCommandType = ToolCommandTypeCloseGateIncrementally;
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
	for (int i = 0; i < TOOL_COUNT; i++) {

		if (digitalRead(tools[i].togglePin) == LOW) {

			toggleDustCollector();

			if (!tools[i].open) {
				closeAllGates(i);
				openGate(i);
			}

			break;
		}

	}
}

void openAllGates() {
	Serial.println("Closing all tools");

	for(int i = 0; i < TOOL_COUNT; i++){
		if(!tools[i].open){
			openGate(i);
		}
	}
}

void openGate(int index) {
	Serial.println("Opening tool " + index);

	printToLcd("Opening Gate " + String(index) + " for the " + tools[index].name, "");
	tools[index].open = true;

	long newPosition = tools[index].stepper.currentPosition() + (STEPS_PER_REVOLUTION * REVOLUTIONS_PER_CYCLE);
	moveGate(index, newPosition);
}

void openGateIncrementally(int index, int value) {
	int revolutions = getRevolutions(value);
	Serial.println("Opening tool " + String(index) + " amount: " + String(revolutions));
	long newPosition = tools[index].stepper.currentPosition() + (STEPS_PER_REVOLUTION * revolutions);
	moveGate(index, newPosition);
}

void closeAllGates(int index) {
	Serial.println("Closing all tools");

	for(int i = 0; i < TOOL_COUNT; i++){
		if(index != i && tools[i].open){
			closeGate(i);
		}
	}
}

void closeGate(int index) {
	Serial.println("Closing tool " + index);
	tools[index].open = false;
	long newPosition = tools[index].stepper.currentPosition() - (STEPS_PER_REVOLUTION * REVOLUTIONS_PER_CYCLE);
	moveGate(index, newPosition);
}

void closeGateIncrementally(int index, int value) {
	int revolutions = getRevolutions(value);
	Serial.println("Opening tool " + String(index) + " amount: " + String(revolutions));
	long newPosition = tools[index].stepper.currentPosition() - (STEPS_PER_REVOLUTION * revolutions);
	moveGate(index, newPosition);
}

void moveGate(int index, long newPosition) {
	wakeTool(index);
	tools[index].stepper.runToNewPosition(newPosition);
	sleepTool(index);
}

int getRevolutions(int value) {
	if (value == 0) {
		return 1;
	} else {
		return value / 10;
	}

	return 0;
}

void wakeTool(int index) {
	Serial.println("Waking tool " + index);
	digitalWrite(tools[index].sleepPin, HIGH);
}

void sleepTool(int index) {
	Serial.println("Sleeping tool " + index);
	digitalWrite(tools[index].sleepPin, LOW);
}
