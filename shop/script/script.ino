#include <AccelStepper.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

// setup oled display
Adafruit_SSD1306 display(4);

// toggles dust collector
const int DUST_COLLECTOR_PIN = 10;

// MotorInterfaceType
const int EASY_DRIVER_INTERFACE = 1;

// stepper motor steps per single revolution
const int STEPS_PER_REVOLUTION = 1600;

// number of revolutions to open/close blast gate, around 4" of travel
const double REVOLUTIONS_PER_CYCLE = 11.9;

// speeds set for the quickest opening and closing
const int STEPPER_MAX_SPEED = 5000;
const int STEPPER_ACCELERATION = 5000;

// used to determine if a motor exists
const int HOMING_THRESHOLD = -22000;

// to eliminate possible signal noise, require a minimum duration button press
const long BUTTON_PRESS_MINIMUM_DURATION = 100;

// to eliminate possible signal noise, require a minimum duration button press
const long RESET_BUTTON_PRESS_MINIMUM_DURATION = 2000;

// default value read from memory
const int CLEARED_MEMORY_VALUE = 255;

// pin for the reset memory button
const int MEMORY_RESET_PIN = 11;

// number of gates to configure
const int GATE_COUNT = 8;

// function for programatically resetting the arduino
void(* resetArduino) (void) = 0;

// declare stepper motors, interface type, step pin, direction pin
AccelStepper stepper1 (EASY_DRIVER_INTERFACE, 52, 50);
AccelStepper stepper2 (EASY_DRIVER_INTERFACE, 44, 42);
AccelStepper stepper3 (EASY_DRIVER_INTERFACE, 36, 34);
AccelStepper stepper4 (EASY_DRIVER_INTERFACE, 28, 26);
AccelStepper stepper5 (EASY_DRIVER_INTERFACE, 53, 51);
AccelStepper stepper6 (EASY_DRIVER_INTERFACE, 45, 43);
AccelStepper stepper7 (EASY_DRIVER_INTERFACE, 37, 35);
AccelStepper stepper8 (EASY_DRIVER_INTERFACE, 29, 27);

struct Gate {
  // stepper object
  AccelStepper stepper;

  // gates are only enabled if homing switch gets activated
  boolean isEnabled;

  // pin for toggle buttons to open/close gate
  int togglePin;

  // pin for homing switch
  int homingPin;

  // pin to control sleep/wake
  int sleepPin;

  // is stepper currently open
  boolean isOpen;
};

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

  pinMode(MEMORY_RESET_PIN, INPUT_PULLUP);
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
  delay(350);
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

    // open gate to max open position
    while (homingSwitchIsNotActive(i)) {
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

    // once max open position is aquired, the motor has to decelerate
    // backoff switch until deactivated
    while (homingSwitchIsActive(i)) {
      gates[i].stepper.moveTo(initialPosition);
      gates[i].stepper.run();
      initialPosition++;
      delay(1);
    }

    Serial.println("Homing switch deactivated");

    // reset gate values
    gates[i].isEnabled = true;
    gates[i].isOpen = true;
    gates[i].stepper.setCurrentPosition(0);
    sleepGate(i);

    closeGate(i);
  }

  writeGatesToMemory();

  Serial.println("Homing procedure complete");
}

void detectGateButtonPress() {
  boolean buttonPressActive = false;
  long buttonPressTimer = 0;

  for (int i = 0; i < GATE_COUNT; i++) {
    buttonPressActive = false;

    while (toggleButtonIsPressed(i)) {
      if (!buttonPressActive) {
        buttonPressActive = true;
        buttonPressTimer = millis();
      }

      if ((millis() - buttonPressTimer) > BUTTON_PRESS_MINIMUM_DURATION) {
        activateGate(i);
        break;
      }
    }
  }
}

void detectResetButtonPress() {
  boolean buttonPressActive = false;
  long buttonPressTimer = 0;

  while (memoryResetButtonIsPressed()) {
    if (!buttonPressActive) {
      buttonPressActive = true;
      buttonPressTimer = millis();
    }

    if ((millis() - buttonPressTimer) > RESET_BUTTON_PRESS_MINIMUM_DURATION) {
      resetSystem();
      break;
    }
  }
}

void activateGate(int index) {
  clearMemory();
  toggleDustCollector();
  openGate(index);
  closeAllGates(index);
  printToDisplay("This gate is open", index);
  writeGatesToMemory();
  preventRetoggle();
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

void preventRetoggle() {
  // prevent a retoggle by delaying to allow user to release toggle button
  delay(1000);
}

bool toggleButtonIsPressed(int index) {
  return digitalRead(gates[index].togglePin) == LOW;
}

bool memoryResetButtonIsPressed() {
  return digitalRead(MEMORY_RESET_PIN) == LOW;
}

bool homingSwitchIsActive(int index) {
  return digitalRead(gates[index].homingPin) == LOW;
}

bool homingSwitchIsNotActive(int index) {
  return digitalRead(gates[index].homingPin) == HIGH;
}

void clearMemory() {
  Serial.println("Resetting arduino EEPROM...");

  for (int i = 0 ; i < GATE_COUNT * 2; i++) {
    EEPROM.write(i, CLEARED_MEMORY_VALUE);
  }

  Serial.println("EEPROM cleared...");
}

void writeGatesToMemory() {
  Serial.println("Saving gates to memory...");

  for (int i = 0; i < GATE_COUNT; i++) {
    writeGateToMemory(i);
  }

  Serial.println("Gates saved...");
}

void readGatesFromMemory() {
  Serial.println("Saving gates to memory...");

  for (int i = 0; i < GATE_COUNT; i++) {
    readGateFromMemory(i);
  }

  Serial.println("Gates saved...");
}

void writeGateToMemory(int index) {
  EEPROM.write(index, gates[index].isEnabled ? 1 : 0);
  EEPROM.write(index + GATE_COUNT, gates[index].isOpen ? 1 : 0);
}

void readGateFromMemory(int index) {
  gates[index].isEnabled = EEPROM.read(index) == 1;
  gates[index].isOpen = EEPROM.read(index + GATE_COUNT) == 1;
}

bool gatesAreSaved() {
  if (EEPROM.read(0) == CLEARED_MEMORY_VALUE) {
    Serial.println("Gates are not saved in memory...");
    return false;
  }

  Serial.println("Gates are saved in memory...");
  readGatesFromMemory();

  return true;
}

void resetSystem() {
  printToDisplay("Resetting System...", -1);
  clearMemory();
  resetArduino();
}

void setup() {
  initializeSerial();
  initializeDisplay();
  initializeStepperMotors();
  initializeToggleButtons();
  initializeHomingSwitches();
  initializeDustCollector();

  delay(250);

  if (!gatesAreSaved()) {
    beginHomingProcedure();
  }

  printToDisplay("System Ready...", -1);
}

void loop() {
  detectGateButtonPress();
  detectResetButtonPress();
}
