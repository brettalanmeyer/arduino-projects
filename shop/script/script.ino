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

enum ToolSelectionState {
	SelectTool,
	SelectCommand,
	SelectValue
};

enum ToolCommandType {
	Open,
	Close,
	OpenX,
	CloseX
};

ToolSelectionState currentToolSelectionState = ToolSelectionState.SelectTool;
ToolCommandType currentToolCommandType = NULL;
int selectedToolNumber = NULL;


// toggles power to lcd backlight
const int LCD_TRANSISTOR_PIN = 13;

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
	checkKeypad();
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
	printToLcd("Welcome to Brett's Shop", "Select a tool: 1 - 8");
}

void printToLcd(String line1, String line2) {
	lcd.clear();

	lcd.setCursor(0, 0);
	lcd.print(line1);

	if (line2 != NULL) {
		lcd.setCursor(0, 1);
		lcd.print(line2);
	}

	turnLcdBackLightOn();
	lcdTurnOffDelay = millis() / 1000;
}

void updateLcd() {
	if(lcdBackLightOn && ((millis() / 1000) - lcdTurnOffDelay > 5)){
		turnLcdBackLightOff();
	}
}

void checkKeyPad() {

	char key = keypad.getKey();

	if (key) {

		if (isNumber(key)) {

			if (currentToolSelectionState == ToolSelectionState.SelectTool) {
				currentToolSelectionState = ToolSelectionState.SelectCommand;
				selectedToolNumber = (int) key;

				printToLcd("A: Open     B: Close", "C: Open x   D: Close X");

			} else if(currentToolSelectionState == ToolSelectionState.SelectValue) {
				selectedValue = (int) key;

				if(currentToolCommandType == ToolCommandType.OpenX){
					openToolX(selectedToolNumber - 1, selectedValue);
				} else if(currentToolCommandType == ToolCommandType.CloseX) {
					closeToolX(selectedToolNumber - 1, selectedValue);
				}

			}

		} else if(isControl(key)) {

			currentToolSelectionState == ToolSelectionState.SelectTool
			selectedToolNumber = NULL;
			selectedValue = NULL;
			printDefaultMessage();

		} else if (isLetter(key)) {

			if (currentToolSelectionState == ToolSelectionState.SelectCommand) {
				if (key == 'A') {
					currentToolCommandType = ToolCommandType.Open;
					openTool(selectedToolNumber - 1);
				} else if (key == 'B') {
					currentToolCommandType = ToolCommandType.Close;
					closeTool(selectedToolNumber - 1);
				} else if (key == 'C') {
					currentToolCommandType = ToolCommandType.OpenX;
				} else if (key == 'D') {
					currentToolCommandType = ToolCommandType.CloseX;
				}

			}

		}

	}

}

boolean isLetter(char c) {
	return (c == 'A' || c == 'B' || c == 'C' || c == 'D');
}

boolean isControl(char c) {
	return (c == '*' || c == '#');
}

boolean isNumber(char c) {
	return (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == '0');
}

void checkToggleButton() {
	for(int i = 0; i < TOOL_COUNT; i++){

		if(digitalRead(tools[i].button) == LOW){

			toggleDustCollector();

			if (!tools[i].open) {
				closeAllTools(i);
				openTool(i);
			}

			break;
		}

	}
}

void openTool(int index) {
	Serial.println("Opening tool " + index);

	printToLcd("Opening Gate " + index + " for the " tools[index].name, NULL);
	tools[index].open = true;

	long newPosition = tools[index].stepper.currentPosition() + (STEPS_PER_REVOLUTION * REVOLUTIONS_PER_CYCLE);
	moveTool(index, newPosition);
}

void openToolX(int index, int value) {
	int revolutions = getRevolutions(value);
	Serial.println("Opening tool " + index + " amount: " + revolutions);
	long newPosition = tools[index].stepper.currentPosition() + (STEPS_PER_REVOLUTION * revolutions);
	moveTool(index, newPosition);
}

void closeTool(int index) {
	Serial.println("Closing tool " + index);
	tools[index].open = false;
	long newPosition = tools[index].stepper.currentPosition() - (STEPS_PER_REVOLUTION * REVOLUTIONS_PER_CYCLE);
	moveTool(index, newPosition);
}

void closeToolX(int index, int value) {
	int revolutions = getRevolutions(value);
	Serial.println("Opening tool " + index + " amount: " + revolutions);
}

void moveTool(int index, long newPosition) {
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

void closeAllTools(int index) {
	Serial.println("Closing all tools");

	for(int i = 0; i < TOOL_COUNT; i++){
		if(index != i && tools[i].open){
			closeTool(i);
		}
	}
}

void wakeTool(int index) {
	Serial.println("Waking tool " + index);
	digitalWrite(tools[index].sleepPin, HIGH);
}

void sleepTool(int index) {
	Serial.println("Sleeping tool " + index);
	digitalWrite(tools[index].sleepPin, LOW);
}


