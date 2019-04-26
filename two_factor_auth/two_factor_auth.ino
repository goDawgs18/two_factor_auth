/*
 * https://github.com/miguelbalboa/rfid
 * --------------------------------------
 *             MFRC522      Arduino       
 *             Reader/PCD   Uno/101      
 * Signal      Pin          Pin          
 * --------------------------------------
 * RST/Reset   RST          9             
 * SPI SS      SDA(SS)      10           
 * SPI MOSI    MOSI         11
 * SPI MISO    MISO         12 
 * SPI SCK     SCK          13
*/

// Using to communicate and handle RFID module
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

const byte ROWS = 3; 
const byte COLS = 3; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'}
};

byte rowPins[ROWS] = {19, 18, 17}; 
byte colPins[COLS] = {16, 15, 14}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

int matchedRFID;

// This will let us know which LEDs are lit
int LED_STATE;

volatile uint8_t delayTime;

int checkRFID;
int timeoutRFID;

char nums[4];
int keyIdx;

unsigned long prevMillis = 0;


void setup() {
  keyIdx = 0;
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  
  // put your setup code here, to run once:
  //set the digital pins 2-7 to outputs
  DDRD = DDRD  | 0b11111100;

  //making sure that digital pin 8 is an input and others are out
  DDRB &= ~1;

  //sets all the analog pins for input for pin pad
  DDRC &= 0;
  
  delayTime = 100;
  timeoutRFID = 0;
  //setting the first bit PCIE0 on to enable PCINT0
  PCICR |= 1;

  //making sure that the Digital pin 8 throws interrupt
  PCMSK0 |= 1;
  
  checkRFID = false;
}


ISR (PCINT0_vect) {
  checkRFID = true;
  Serial.println(F("RFID ISR"));
}

void readRFID() {
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  Serial.println(F("readRFID()"));
  
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  if (mfrc522.uid.uidByte[0] == 0xDB &&
        mfrc522.uid.uidByte[1] == 0xF6 &&
        mfrc522.uid.uidByte[2] == 0xF0 &&
        mfrc522.uid.uidByte[3] == 0x0A) {
    Serial.println(F("APPROVED USERS RFID SCANNED"));
    matchedRFID = true;
  } else {
    Serial.println(F("UNAPPROVED USER ATTEMPTED SCAN"));
    matchedRFID = false;
  }
}

void checkCode() {
  if (nums[0] == 1 &&
        nums[1] == 2 &&
        nums[2] == 3 &&
        nums[3] == 4) {
      PORTD |= 1 <<3;
  }
}

void loop() {
  char customKey = customKeypad.getKey();
  
  if (customKey){
    nums[keyIdx] = customKey;

    if (keyIdx <= 2) {
      keyIdx++;
    } else {
      keyIdx = 0;
    }

    Serial.println(keyIdx);
    checkCode();
    Serial.println(customKey);
  }


  
  if (checkRFID) {
    PCMSK0 &= ~1;
    readRFID();
    checkRFID = false;
    PCMSK0 |= 1;
  }
  
  //because 0 and 1 are not being used
  PORTD = matchedRFID << 2;
}

void myTimer() {
  unsigned long currMillis = millis();
  while ((currMillis - prevMillis) < delayTime) {
    currMillis = millis();
  }
  prevMillis = currMillis;
}
