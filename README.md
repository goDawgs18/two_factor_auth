# two_factor_auth
In this program I plan on using a RFID key fob and a pin as a two factor authorization process for users to access a secret code

## How to use it
1. First scan the permitted RFID card
1. Once you get an Approved card message you will need to enter your pin
1. Enter your pin
	1. Psst... the pin is 1234
1. Once you have entered the correct pin it will grant you access and you can see the secret code


## Libraries used
* MFRC522
	* This library is used to read the RFID cards
* SPI
	* This is used to communicate with the RFID reader
* Keypad
	* This library is used to handle the keypad
* LiquidCrystal
	* This library controlls the LCD 1602 screen

## Future work
* Password encryption
	* Currently the UID and pin used in the program are not encrypted. In the future the inputs should be compared to the encrypted version of these keys
* More interesting ending

## Code Constants
* WRONG_PIN_ATTEMPT_LIMIT
	* This is the number of times a user can get the pin wrong till they have to go back to the start
* DISPLAY_TRANSITION
	* This controls how long the display shows the transition statements (EX Card approved, pin approved)

## Connections

RFID-RC522 | Arduino Uno
------------ | -------------
3.3V		|		3.3V
RST			|		9
GND			|		GND
IRQ			|		8
MISO		|		12
MOSI		|		11
SCK			|		13
SDA			|		10

4x4 Pin Pad | Arduino Uno
------------ | -------------
R1		|		A5
R2		|		A4
R3		|		A3
R4		|		NOTHING
C1		|		A2
C2		|		A1
C3		|		A0
C4		|		NOTHING

LCD 1602	|	Arduino Uno
------------ | -------------
VSS		|		GND
VDD		|		5V
V0		|		10K POT
RS		|		2
RW		|		GND
E		|		3
D0		|		NOTHING
D1		|		NOTHING
D2		|		NOTHING
D3		|		NOTHING
D4		|		4
D5		|		5
D6		|		6
D7		|		7
A		|		3.3V
K		|		GND


## Known Issues
* The UID from an RFID card can be spoofed
* Can only read Cards default key
* Can find the passwords easily in the code

