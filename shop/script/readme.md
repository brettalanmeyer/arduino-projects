# Blast Gate project

## Resources

* http://www.airspayce.com/mikem/arduino/AccelStepper/annotated.html
* https://brainy-bits.com/blogs/tutorials/setting-stepper-motors-home-position-using-accelstepper
* https://brainy-bits.com


Open All Gates
Close All Gates


## Prompts

All gates start off closed
Program initializes
LCD


	Welcome to Brett's Shop
	Select a tool: 1 - 8
	>> 2
	A: Open     B: Close
	C: Open x   D: Close X
	>> A
	Opening Gate 2 for the Drill Press


	Welcome to Brett's Shop
	Select a tool: 1 - 8
	>> 8
	A: Open		B: Close
	C: Open x	D: Close X
	>> B
	Closing Gate 8 for the Table Saw


	Welcome to Brett's Shop
	Select a tool: 1 - 8
	>> 6
	A: Open		B: Close
	C: Open x	D: Close X
	>> C
	Select a number
	>> 1
	Opening Gate 6 for the Band Saw 1/10 revolution


	Welcome to Brett's Shop
	Select a tool: 1 - 8
	>> 6
	A: Open		B: Close
	C: Open x	D: Close X
	>> C
	Select a number
	>> 5
	Opening Gate 6 for the Band Saw 5/10 revolution


	Welcome to Brett's Shop
	Select a tool: 1 - 8
	>> 6
	A: Open		B: Close
	C: Open x	D: Close X
	>> C
	Select a number
	>> 0
	Opening Gate 6 for the Band Saw 1 revolution





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