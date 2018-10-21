#include <AccelStepper.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// setup oled display
Adafruit_SSD1306 display(4);

// toggles dust collector
const int DUST_COLLECTOR_PIN = 10;

// MotorInterfaceType
const int EASY_DRIVER_INTERFACE = 1;

// stepper motor steps per single revolution
const int STEPS_PER_REVOLUTION = 1600;

// number of revolutions to open/close blast gate, around 4" of travel
const double REVOLUTIONS_PER_CYCLE = 11.7;

// speeds set for the quickest opening and closing
const int STEPPER_MAX_SPEED = 5000;
const int STEPPER_ACCELERATION = 5000;

// used to determine if a motor exists
const int HOMING_THRESHOLD = -20000;

// declare stepper motors, interface type, step pin, direction pin
AccelStepper stepper1 (EASY_DRIVER_INTERFACE, 52, 50);
AccelStepper stepper2 (EASY_DRIVER_INTERFACE, 44, 42);
AccelStepper stepper3 (EASY_DRIVER_INTERFACE, 36, 34);
AccelStepper stepper4 (EASY_DRIVER_INTERFACE, 28, 26);
AccelStepper stepper5 (EASY_DRIVER_INTERFACE, 53, 51);
AccelStepper stepper6 (EASY_DRIVER_INTERFACE, 45, 43);
AccelStepper stepper7 (EASY_DRIVER_INTERFACE, 37, 35);
AccelStepper stepper8 (EASY_DRIVER_INTERFACE, 29, 27);

// pins and stepper motors for toggling, waking, and sleeping motors
struct Gate {
  // stepper object
  AccelStepper stepper;

  // gates are only enabled if bumper switch is triggered during homing process
  boolean isEnabled;

  // pin for toggle buttons to open/close gate
  int togglePin;

  // pin for homing bumper switch
  int homingPin;

  // pin to control sleep/wake
  int sleepPin;

  // is stepper currently open
  boolean isOpen;
};

// number of gates to configure
const int GATE_COUNT = 8;

// define pins and steppers for each gate
// order indicates which patch panel port the gate is connected to
Gate gates[GATE_COUNT] = {
  { stepper5, false, 2, 49, 47, false },
  { stepper6, false, 3, 41, 39, false },
  { stepper7, false, 4, 33, 31, false },
  { stepper8, false, 5, 25, 23, false },
  { stepper1, false, 7, 48, 46, false },
  { stepper2, false, 6, 40, 38, false },
  { stepper3, false, 8, 32, 30, false },
  { stepper4, false, 9, 24, 22, false },
};

void initializeSerial() {
  Serial.begin(9600);
  delay(50);
  Serial.println("Beginning setup...");
}

void initializeDisplay() {
  // generate the high voltage from the 3.3v line internally
  // initialize with the I2C addr 0x3C (for the 128x32)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // show image buffer on the display hardware
  // since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen
  display.display();
  
  delay(1000);

  // clear the buffer
  display.clearDisplay();

  // set default text color
  display.setTextColor(WHITE);
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
    gates[i].stepper.setMaxSpeed(STEPPER_MAX_SPEED);  
    gates[i].stepper.setAcceleration(STEPPER_ACCELERATION);
    pinMode(gates[i].sleepPin, OUTPUT);
  }
}

void initializeDustCollector() {
  Serial.println("Initializing dust collector...");
 
  pinMode(DUST_COLLECTOR_PIN, OUTPUT);
}

void initializeHomingSwitches() {
  Serial.println("Initializing homing switches...");
  
  for (int i = 0; i < GATE_COUNT; i++) {
    pinMode(gates[i].homingPin, INPUT_PULLUP);
  }
}

void toggleDustCollector() {
  Serial.println("Toggling dust collector remote");
  
  digitalWrite(DUST_COLLECTOR_PIN, HIGH);
  delay(250);
  digitalWrite(DUST_COLLECTOR_PIN, LOW);
}

void beginHomingProcedure() {
  Serial.println("Beginning homing procedure...");
  
  delay(250);

  for (int i = 0; i < GATE_COUNT; i++) {
    Serial.print("Homing gate ");
    Serial.println(i);   
    printToDisplay("Homing...", i);

    long initialPosition = -1;
    bool gateExists = true;
    
    wakeGate(i);
    
    // open until switch is activated
    while (digitalRead(gates[i].homingPin) == HIGH) {
      gates[i].stepper.moveTo(initialPosition);
      gates[i].stepper.run();
      initialPosition--;
      delayMicroseconds(250);

      // switch was never activated which means gate is not connected
      if (initialPosition < HOMING_THRESHOLD){
        gateExists = false;
        break;
      }
    }

    if (!gateExists) {
      sleepGate(i);
      continue;  
    }

    Serial.println("Homing switch activated");

    // backoff switch until is deactivated
    while (digitalRead(gates[i].homingPin) == LOW) {
      gates[i].stepper.moveTo(initialPosition);
      gates[i].stepper.run();
      initialPosition++;
      delay(1);
    }

    Serial.println("Homing switch deactivated");

    gates[i].isEnabled = true;
    gates[i].isOpen = true;
    gates[i].stepper.setCurrentPosition(0);
    sleepGate(i);

    closeGate(i);
  }

  Serial.println("Homing procedure complete");  
  printToDisplay("System Ready...", -1);
}

void checkToggleButton() {
  for (int i = 0; i < GATE_COUNT; i++) {
    if (digitalRead(gates[i].togglePin) == LOW) {
      toggleDustCollector();
      openGate(i);
      closeAllGates(i);    
      printToDisplay("This gate is open", i);
      break;
    }
  }
}

void openGate(int index) {
  Serial.print("Opening gate ");
  Serial.println(index); 
  
  if(!gates[index].isEnabled) return;
  if(gates[index].isOpen) return;

  printToDisplay("Opening...", index);
  
  gates[index].isOpen = true;

  long newPosition = gates[index].stepper.currentPosition() - (STEPS_PER_REVOLUTION * REVOLUTIONS_PER_CYCLE);
  moveGate(index, newPosition);
}

void closeAllGates(int index) {
  Serial.println("Closing all gates");

  for(int i = 0; i < GATE_COUNT; i++){
    if(index != i){
      closeGate(i);
    }
  }
}

void closeGate(int index) {
  Serial.print("Closing gate ");
  Serial.println(index);

  if(!gates[index].isEnabled) return;
  if(!gates[index].isOpen) return;
  
  printToDisplay("Closing...", index);
  
  gates[index].isOpen = false;
  long newPosition = gates[index].stepper.currentPosition() + (STEPS_PER_REVOLUTION * REVOLUTIONS_PER_CYCLE);
  moveGate(index, newPosition);
}

void moveGate(int index, long newPosition) {
  wakeGate(index);
  gates[index].stepper.runToNewPosition(newPosition);
  sleepGate(index);
}

void wakeGate(int index) {
  Serial.print("Waking gate ");
  Serial.println(index);
 
  digitalWrite(gates[index].sleepPin, HIGH);
}

void sleepGate(int index) {
  Serial.print("Sleeping gate ");
  Serial.println(index);
 
  digitalWrite(gates[index].sleepPin, LOW);
}

void printToDisplay(String line, int gate) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(line);

  if (gate != -1) {
    display.setTextSize(3);
    display.print("Gate ");
    display.println(gate + 1);
  }
  
  display.display();
}

void setup() {
  initializeSerial();
  initializeDisplay();
  initializeStepperMotors();
  initializeToggleButtons();
  initializeHomingSwitches();
  initializeDustCollector();

  delay(250);

  beginHomingProcedure();
}

void loop() {
  checkToggleButton();
}
