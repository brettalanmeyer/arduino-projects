#include <Servo.h>
#include <LiquidCrystal.h>

struct Tool {
	String name;
	Servo servo;
	boolean open;
	int pin;
	int button;
};

const int TOOL_COUNT = 7;

Servo jointerServo;
Servo routerTableServo;
Servo bandSawServo;
Servo tableSawServo;
Servo miterSawServo;
Servo drillPressServo;
Servo planerServo;

Tool tools[TOOL_COUNT] = {
	{ "Jointer", jointerServo, true, 2, 53 },
	{ "Router Table", routerTableServo, true, 3, 51 },
	{ "Band Saw", bandSawServo, true, 4, 49 },
	{ "Table Saw", tableSawServo, true, 5, 47 },
	{ "Miter Saw", miterSawServo, true, 6, 45 },
	{ "Drill Press", drillPressServo, true, 7, 43 },
	{ "Planer", planerServo, true, 8, 41 }
};

// LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
	do_setup();
	setup_buttons();
	setup_servos();
	begin_loop();
}

void loop() {
	check_for_button_press();
}

void do_setup() {
	Serial.begin(9600);

	// lcd.begin(16, 2);
	pinMode(LED_BUILTIN, OUTPUT);

	log("Starting up...");
}

void setup_buttons() {
	log("Setting up buttons...");

	for(int i = 0; i < TOOL_COUNT; i++){
		pinMode(tools[i].button, INPUT_PULLUP);
	}
}

void setup_servos() {
	log("Setting up servos...");

	for(int i = 0; i < TOOL_COUNT; i++){
		tools[i].servo.attach(tools[i].pin);
	}
}

void begin_loop() {
	close_all(-1);
	log("Starting loop...");
}


void check_for_button_press() {
	for(int i = 0; i < TOOL_COUNT; i++){
		if(digitalRead(tools[i].button) == LOW && !tools[i].open){
			close_all(i);
			open(i);
			break;
		}
	}
}

void open(int index) {
	log(tools[index].name + " has been pressed and will be opening");
	tools[index].open = true;
	tools[index].servo.write(10);
}

void close(int index) {
	log(tools[index].name + " is open and will be closing");
	tools[index].open = false;
	tools[index].servo.write(170);
}

void close_all(int index) {
	for(int i = 0; i < TOOL_COUNT; i++){
		if(index != i && tools[i].open){
			close(i);
			delay(500);
		}
	}
}

// void toggle_infrared() {

// }


void log(String string) {
	Serial.println(string);

	// lcd.clear();
	// lcd.setCursor(0, 0);

	// if(string.length() <= 16){
	// 	lcd.print(string);
	// } else {
	// 	lcd.print(string.substring(0, 16));
	// 	lcd.setCursor(0, 1);
	// 	lcd.print(string.substring(16, string.length()));
	// }

	// delay(1000);
}
