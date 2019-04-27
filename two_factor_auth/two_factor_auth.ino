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
#include <LiquidCrystal.h>

#define LCD_RS      2
#define LCD_EN      3
#define LCD_D4      4
#define LCD_D5      5
#define LCD_D6      6
#define LCD_D7      7

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

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

int matchedPIN;

int state;

// This will let us know which LEDs are lit
int LED_STATE;

uint8_t delayTime;

int checkRFID;
int timeoutRFID;

char nums[4];
int keyIdx;

unsigned long prevMillis = 0;


void setup() {
  keyIdx = 0;
  Serial.begin(9600); // Initialize serial communications with the PC
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  lcd.begin(16,2);
  askCard();
  
  // put your setup code here, to run once:
  //set the digital pins 2-7 to outputs
//  DDRD = 0b11111100;

  //making sure that digital pin 8 is an input and others are out
  DDRB &= ~1;

  //sets all the analog pins for input for pin pad
  DDRC &= 0;
  
  delayTime = 255;
  timeoutRFID = 0;
  //setting the first bit PCIE0 on to enable PCINT0
  PCICR |= 1;

  //making sure that the Digital pin 8 throws interrupt
  PCMSK0 |= 1;

  matchedRFID = 0;
  matchedPIN = 0;
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
    approvedCard();
    matchedRFID = 1;
    state = 1;
    Serial.println(F("APPROVED USERS RFID SCANNED"));
  } else {
    wrongCard();
    matchedRFID = 0;
    Serial.println(F("UNAPPROVED USER ATTEMPTED SCAN"));
  }
}

void checkCode() {
  if (nums[0] == '1' &&
        nums[1] == '2' &&
        nums[2] == '3' &&
        nums[3] == '4') {
      approvedCard(); 
      matchedPIN = true;
      state = 2;
  } else {
      wrongPin();
      matchedPIN = false;
  }
}

void numsToSerial() {
  Serial.println(F("NUMS:"));
  Serial.print(F("{"));
  for(int i = 0; i < 4; i++) {
    Serial.print(nums[i]);
    Serial.print(F(","));
  }
  Serial.print(F("}"));
  Serial.println();
}

void askCard() {
  lcd.setCursor(0,0);
  lcd.print("HELLO USER,     ");
  lcd.setCursor(0,1);
  lcd.print("PLEASE SCAN CARD");
}

void askPin() {
  lcd.setCursor(0,0);
  lcd.print("Now enter PIN   ");
  lcd.setCursor(0,1);
  char pinStr[16] = "                ";
  for(int i=0; i<keyIdx; i++) {
    pinStr[i] = '*';
  }
  lcd.print(pinStr);
}


void clearScreen() {
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
}

void approvedCard() {
  lcd.setCursor(0,0);
  lcd.print(" APPROVED  CARD ");
  lcd.setCursor(0,1);
  lcd.print("  STAGE 1 DONE  ");
  myTimer();
  Serial.println(F("Approved Card Scanned"));
}

void approvedPin() {
  lcd.setCursor(0,0);
  lcd.print("  CORRECT  PIN  ");
  lcd.setCursor(0,1);
  lcd.print("  STAGE 2 DONE  ");
  myTimer();
  Serial.println(F("Approved pin Entered"));
}

void wrongCard() {
  lcd.setCursor(0,0);
  lcd.print("   WRONG CARD   ");
  lcd.setCursor(0,1);
  lcd.print("  RE-SCAN CARD  ");
  myTimer();
  Serial.println("Wrong Card");
  askCard();
}

void wrongPin() {
  lcd.setCursor(0,0);
  lcd.print("   WRONG  PIN   ");
  lcd.setCursor(0,1);
  lcd.print("  RE-ENTER PIN  ");
  myTimer();
  Serial.println("Wrong Pin");
}

void grantAccess() {
  lcd.setCursor(0,0);
  lcd.print("GRANTED ACCESS  ");
  lcd.setCursor(0,1);
  lcd.print("PLS HIRE ME     ");
}

void loop() {
  char customKey = customKeypad.getKey();
  
  if (checkRFID) {
    PCMSK0 &= ~1;
    readRFID();
    checkRFID = false;
    PCMSK0 |= 1;
    //if it found one then it should never run again
  }
  
  switch (state) {
    case 1:
      PCICR &= ~1;
      askPin();
      if (customKey){
        nums[keyIdx] = customKey;
        Serial.print(F("Value Pressed: "));
        Serial.println(customKey);
    
        Serial.print(F("keyIdx is currently:"));
        Serial.println(keyIdx);
    
        numsToSerial();
        if (keyIdx <= 2) {
          keyIdx++;
        } else {
          checkCode();
          keyIdx = 0;
        }
    
        Serial.println(customKey);
      }
      break;
     case 2:
       grantAccess();
       break;     
  }
  
}

void myTimer() {
  unsigned long currMillis = millis();
  while ((currMillis - prevMillis) < 8000) {
    currMillis = millis();
  }
  prevMillis = currMillis;
}

