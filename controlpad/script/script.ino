// include the library code:
#include <LiquidCrystal.h>
#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};

// connect to the row pinouts of the keypad starting with left most pin 8, 7, 6, 5
byte rowPins[ROWS] = { 29, 27, 25, 23 };
// connect to the column pinouts of the keypad ending with right most pin 4, 3, 2, 1
byte colPins[COLS] = { 3, 4, 5, 6 };

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

const int transistorPin = 13;

boolean lcdBacklightOn = false;
int lcdTurnOffDelay = millis();

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);


void setup() {
	Serial.begin(9600);

	pinMode(transistorPin, OUTPUT);

	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);

	// Print a message to the LCD.
	lcd.print("hello, world!");
	turnLcdBacklightOn();
}

void loop() {

	char key = keypad.getKey();

	if (key){
		printToLcd(String(key));
		Serial.println(key);
	}

	updateLcd();

}

void turnLcdBacklightOn() {
	lcd.display();
	digitalWrite(transistorPin, HIGH);
	lcdBacklightOn = true;
}

void turnLcdBacklightOff() {
	lcd.noDisplay();
	digitalWrite(transistorPin, LOW);
	lcdBacklightOn = false;
}

void printToLcd(String value) {
	lcd.clear();
	lcd.print(value);
	turnLcdBacklightOn();
	lcdTurnOffDelay = millis() / 1000;
}

void updateLcd() {
	if(lcdBacklightOn && ((millis() / 1000) - lcdTurnOffDelay > 5)){
		turnLcdBacklightOff();
	}
}



enum MenuSelectionState {
	Default,
	PageOne,
	PageTwo,
	PageThree
};

enum ToolCommandType {
	OpenAllGates,
	CloseAllGates,
	OpenGateNumber,
	CloseGateNumber,
	OpenGateIncrementally,
	CloseGateIncrementally
};

MenuSelectionState menuSelectionState = MenuSelectionState.Default;
ToolCommandType toolCommandType = NULL;