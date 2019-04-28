/*
 * Author: Alex Castro
 */
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

/*------------------------------
 * This is the setup for the LCD module
 * -----------------------------
 */
#define LCD_RS      2
#define LCD_EN      3
#define LCD_D4      4
#define LCD_D5      5
#define LCD_D6      6
#define LCD_D7      7

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

/*------------------------------
 * END LCD Set up
 * -----------------------------
 * -----------------------------
 * This is the setup for the RFID
 * -----------------------------
 */

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

/*------------------------------
 * END RFID Set up
 * -----------------------------
 * -----------------------------
 * This is the setup for the Key Pad
 * -----------------------------
 */

const byte ROWS = 3; 
const byte COLS = 3; 

char HEX_KEYS[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'}
};

byte ROW_PINS[ROWS] = {19, 18, 17}; 
byte COL_PINS[COLS] = {16, 15, 14}; 

Keypad customKeypad = Keypad(makeKeymap(HEX_KEYS), ROW_PINS, COL_PINS, ROWS, COLS); 

/*------------------------------
 * END Key Pad Set up
 * -----------------------------
 * -----------------------------
 * These are values used in the program
 * -----------------------------
 */
 
#define DISPLAY_TRANSITION        1000
#define WRONG_PIN_ATTEMPT_LIMIT   2

// This will keep track of where we are in the procedure
int STATE;

int pinAttempts;  //Keeps track of the # of wrong pin attempts
char nums[4];     //Holds the pin characters entered
int keyIdx;       //Keeps track of where we are in the pin sequence

// This will be set by the ISR and let us 
// know when the RFID module needs to be read
int checkRFID;



void setup() {
  keyIdx = 0;
  Serial.begin(9600); // Initialize serial communications with the PC
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  lcd.begin(16,2);
  askForCard();

  //making sure that digital pin 8 is an input and others are out
  DDRB &= ~1;

  //sets all the analog pins for input for pin pad
  DDRC &= 0;
  
  //setting the first bit PCIE0 on to enable PCINT0
  PCICR |= 1;

  //making sure that the Digital pin 8 throws interrupt
  PCMSK0 |= 1;

  checkRFID = false;
  pinAttempts = 0;
}

//Flags when we need to check the RFID module
ISR (PCINT0_vect) {
  checkRFID = true;
  Serial.println(F("RFID ISR"));
}

//Reads the RFID module
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
    STATE = 1;
  } else {
    wrongCard();
  }
}

void checkCode() {
  if (nums[0] == '1' &&
        nums[1] == '2' &&
        nums[2] == '3' &&
        nums[3] == '4') {
      approvedPin(); 
      STATE = 2;
  } else {
      pinAttempts++;
      if (pinAttempts >= 2) {
        STATE = 3;
      } else {
        wrongPin();
      }
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

void askForCard() {
  lcd.setCursor(0,0);
  lcd.print("HELLO USER,     ");
  lcd.setCursor(0,1);
  lcd.print("PLEASE SCAN CARD");
}

void askForPin() {
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
  askForCard();
}

void wrongPin() {
  lcd.setCursor(0,0);
  lcd.print("   WRONG  PIN   ");
  lcd.setCursor(0,1);
  lcd.print("  RE-ENTER PIN  ");
  myTimer();
  Serial.println("Wrong Pin");
}

void backToStart() {
  lcd.setCursor(0,0);
  lcd.print("BAD CREDENTIALS ");
  lcd.setCursor(0,1);
  lcd.print("BACK TO START   ");
  myTimer();
}

void grantAccess() {
  lcd.setCursor(0,0);
  lcd.print("GRANTED ACCESS  ");
  lcd.setCursor(0,1);
  lcd.print("PLEASE HIRE ME  ");
}

void loop() {
  char customKey = customKeypad.getKey();

  //This will check if the RFID needs to be read
  if (checkRFID) {
    //Turns off the interrupt trigger
    PCMSK0 &= ~1;
    readRFID();
    //Turns off the flag to check
    checkRFID = false;
    //Activates the interrupt again
    PCMSK0 |= 1;
  }
  
  switch (STATE) {
    case 1: // At this point we have a Card but not the pin
      PCICR &= ~1; //Turn off the ISR no reason to check
      askForPin(); 
      if (customKey){
        //Saved the last key input
        nums[keyIdx] = customKey;
        
        if (keyIdx <= 2) {
          keyIdx++;
        } else {
          checkCode();
          keyIdx = 0;
        }
      }
      break;
    case 2:
      grantAccess();
      break;     
    case 3:
      backToStart();
      PCICR |= 1;
      askForCard();
      STATE = 0;
      break;
  }
}

void myTimer() {
  unsigned long prevMillis = millis();
  unsigned long currMillis = millis();
  while ((currMillis - prevMillis) < DISPLAY_TRANSITION) {
    currMillis = millis();
  }
  prevMillis = currMillis;
}

