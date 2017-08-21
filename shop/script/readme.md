# Blast Gate project

## Resources

* http://www.airspayce.com/mikem/arduino/AccelStepper/annotated.html
* https://brainy-bits.com/blogs/tutorials/setting-stepper-motors-home-position-using-accelstepper
* https://brainy-bits.com





## Prompts

All gates start off closed
Program initializes
LCD



Open All Gates
Close All Gates
Open Gate #
Open Gate # Incrementally
Close Gate #
Close Gate # Incrementally



	Brett's Shop
	# for options

	>> #
	1: Open Gates
	2: Close Gates

	>> #
	3: Open Gate #
	4: Close Gate #

	>> #
	5: OpenX Gate #
	6: CloseX Gate #

	>> #
	Brett's Shop
	# for options

	>> 1





# Pin Outs

## LCD pin out
VSS	ground
VDD	+5v
V0	pot middle pin
RS	7
RW	ground
E	8
D0
D1
D2
D3
D4	9
D5	10
D6	11
D7	12
A	+5v
K	transistor right

## Transistor pin out (flat part is front)
left		ground (arduino)
middle	13 (arduino)
right	K (lcd)

## 10k potentiometer (bottom has 2 pins)
left		+5v (arduino)
middle	V0 (lcd)
right	ground (arduino)

## Keypad pinout - left most pin on keypad is 8
8: 29
7: 27
6: 25
5: 23
4: 3
3: 4
2: 5
1: 6