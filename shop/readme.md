# Blast Gate project - Arduino Mega 2560

This script controls the opening and closing of dust collection blast gates
using stepper motors, flexible couplings, ACME lead screws, and ACME lead screw nuts.

Motors: Stepper Motor Nema 17 Bipolar 40mm 64oz.in(45Ncm) 2A 4 Lead 3D
Control Board: Gikfun EasyDriver Shield Stepper Motor Driver V44 A3967 For Arduino EK1204

## Wiring for Arduino Mega 2560

Connections and cabling will be made using RJ45 and RJ11 ports.

### Arduino Mega Pinouts

|            | AccelStepper Pin 1 | AccelStepper Pin 2 | Toggle Pin | Sleep Pin  | Homing Pin | Port |
|------------|--------------------|--------------------|------------|------------|------------|------|
| Motor 1    | 52                 | 50                 | 6          | 46         | 48         | 5    |
| Motor 2    | 44                 | 42                 | 7          | 38         | 40         | 6    |
| Motor 3    | 36                 | 34                 | 8          | 30         | 32         | 7    |
| Motor 4    | 28                 | 26                 | 9          | 22         | 24         | 8    |
| Motor 5    | 53                 | 51                 | 2          | 47         | 49         | 1    |
| Motor 6    | 45                 | 43                 | 3          | 39         | 41         | 2    |
| Motor 7    | 37                 | 35                 | 4          | 31         | 33         | 3    |
| Motor 8    | 29                 | 27                 | 5          | 23         | 25         | 4    |
| Wire Color | Green              | Green/White        | Blue       | Blue/White | Brown      |      |


#### AccelStepper Pins

**Pin1:** Arduino digital pin number for motor pin 1. Defaults to pin 2. For a AccelStepper::DRIVER (interface==1), this is the Step input to the driver. Low to high transition means to step)

**Pin2:** Arduino digital pin number for motor pin 2. Defaults to pin 3. For a AccelStepper::DRIVER (interface==1), this is the Direction input the driver. High means forward.


### Stepper Motor Pinouts

|            | A+  | A-   | B+    | B-     |
|------------|-----|------|-------|--------|
| Wire Color | Red | Blue | Green | Yellow |


### EasyDriver Pinout

|            | 12v GND     | 12v M+ | GND         | STEP  | DIR         | SLP        |
|------------|-------------|--------|-------------|-------|-------------|------------|
| Wire Color | Orange/White| Orange | Brown/White | Green | Green/White | Blue/White |


### Limit Switch Pinout

|            | GND         | NO   | NC   |
|----------- |-------------|------|------|
| Wire Color | Brown/White | Blue | None |


### Ethernet RJ45 Pinouts

|             | Motor Pin1 | Motor Pin2  | Toggle  | Sleep      | 12v +  | GND          | Limit Switch | None |
|-------------|------------|-------------|---------|------------|--------|--------------|--------------|------|
| Wire Color  | Green      | Green/White | Brown   | Blue/White | Orange | Orange/White | Blue         | None |
| Port Number | 1          | 2           | 3       | 4          | 5      | 6            | 7            | 8    |

### Ethernet RJ11 Pinouts

Use outside two pins. 
|            | GND         | Toggle   |
|----------- |-------------|----------|
| Wire Color | Brown/White | Brown    |

## Images

### Arduino Mega 2560

<img src="arduino-mega-2560.png">

### EasyDriver Shield

<img src="easy-driver.jpg">

### Sample Stepper Motor Wiring Diagram

<img src="stepper-motor-arduino-wiring-diagram.jpg">

## Resources

* http://www.airspayce.com/mikem/arduino/AccelStepper/annotated.html
* https://brainy-bits.com/blogs/tutorials/setting-stepper-motors-home-position-using-accelstepper
* https://brainy-bits.com
