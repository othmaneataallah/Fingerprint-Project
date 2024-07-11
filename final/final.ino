#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>

//  ACCESS/DELETE/ENROLL GLOBALS
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(3, 4);
#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1
#endif
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
bool go = false;

// RELAY GLOBALS
const int relayPin = 8;       // Pin connected to the relay IN pin
const int transistorPin = 13;

// KEYPAD GLOBALS
const int ROW_NUM = 4; // four rows
const int COLUMN_NUM = 4; // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM] = {A0, 12, 11, 10}; // connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {8, 7, 6, 5}; // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// PASSWORD GLOBALS
const String adminPassword = "1234"; // define the admin password
String input = ""; // store the input from the keypad
String currentAction = "ACCESS"; // default action is access

// LCD GLOBALS
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// ENROLL GLOBALS
uint8_t id;

void setup() {
  fingerprintSensorSetup();
  relaySetup();
  lcdSetup();
  transistorSetup();
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    lcd.clear();
    lcd.print(key);
    
    if (currentAction == "ACCESS") {
      if (key == 'B' || key == 'C') {
        currentAction = (key == 'B') ? "ENROLL" : "DELETE";
        input = ""; // clear previous inputs
        lcd.clear();
        lcd.print("Enter Admin Password:");
      } else {
        access();
      }
    } else if (currentAction == "ENROLL" || currentAction == "DELETE") {
      if (key == '#') { // '#' key used to submit the password
        verifyPassword();
      } else if (key == '*') { // '*' key used to clear the input
        input = "";
        lcd.clear();
        lcd.print("Input Cleared");
      } else if (key == 'A') { // 'A' key used to go back to access mode
        currentAction = "ACCESS";
        access();
      } else {
        input += key; // add the pressed key to the input
        lcd.setCursor(0, 1);
        lcd.print(input); // display the current input
      }
    }
  }
}

void access() {
  if (!go){
    Serial.println("Will go");
    lcd.clear();
    lcd.print("Access Mode");
    lcd.setCursor(0, 1);
    lcd.print("Place Fingerprint");
    getFingerprintID();
    go = true;
  }
}

void fingerprintgo() {
 go = false;
}

uint8_t getFingerprintID() {

  uint8_t p = finger.getImage();

  if (imageHandler(p) != FINGERPRINT_OK) return p;

  // OK success!
  p = finger.image2Tz();
  templateHandler(p);

  // OK converted!
  p = finger.fingerSearch();
  searchHandler(p);
  
  // Found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);

  handleSuccess();

  return finger.fingerID;
}

void handleSuccess() {
  ledSuccess();
  printSuccess();
}

void ledSuccess() {
  finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_BLUE);
  delay(1000);
  finger.LEDcontrol(FINGERPRINT_LED_OFF, 0, FINGERPRINT_LED_BLUE);
}

uint8_t imageHandler(uint8_t p) {
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken.");
      return p;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected.");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error.");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error.");
      return p;
    default:
      Serial.println("Unknown error.");
      return p;
  }
}

void templateHandler(uint8_t p) {
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted.");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy.");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error.");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features.");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features.");
      return p;
    default:
      Serial.println("Unknown error.");
      return p;
  }
}

void searchHandler(uint8_t p) {
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match.");
    relayLogic();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error.");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match.");
    handleFailure();
    return p;
  } else {
    Serial.println("Unknown error.");
    return p;
  }
}

void handleFailure() {
  lcdFailure();
  printFailure();
}

void lcdFailure() {
  finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);
}

void relayLogic() {
  

  digitalWrite(relayPin, LOW);
  delay(20);
  digitalWrite(relayPin, HIGH);
  
}

void printSuccess() {
  lcd.setCursor(0,0);
  lcd.print("Welcome!");
  lcd.setCursor(0,1);

  lcd.print("Found a print match!");

  delay(2000);

  for (int scrollCounter = 0; scrollCounter < 20; scrollCounter++) 
  { 

    lcd.scrollDisplayLeft(); 

    delay(400);
  }

  lcd.clear();
}

void printFailure() {
  lcd.setCursor(0,0);
  lcd.print("ARE U EVEN AN ITCIAN?!");
  lcd.setCursor(0,1);

  lcd.print("FUCK OFF!");

  delay(2000);

  for (int scrollCounter = 0; scrollCounter < 22; scrollCounter++) 
  { 

    lcd.scrollDisplayLeft(); 

    delay(800);
  }

  lcd.clear();
}

void enroll() {
  lcd.clear();
  lcd.print("Enrolling");
  lcd.setCursor(0, 1);
  lcd.print("New Fingerprint");
  while(true) { delay(1); }

  // Add your fingerprint enrollment code here
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  id = readnumberKeypad(); // INSTEAD OF: readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);
  while (! getFingerprintEnroll() );
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

void deleteLoop() {
  lcd.clear();
  lcd.print("Deleting");
  lcd.setCursor(0, 1);
  lcd.print("Fingerprint");

  // Add your fingerprint delete code here
  Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");
  uint8_t id = readnumberKeypad(); // INSTEAD OF: readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }
  Serial.print("Deleting ID #");
  Serial.println(id);
  deleteFingerprint(id);
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
  }

  return p;
}

void verifyPassword() {
  if (input == adminPassword) {
    lcd.clear();
    lcd.print("Admin Password");
    lcd.setCursor(0, 1);
    lcd.print("Accepted");
    if (currentAction == "ENROLL") {
      enroll();
    } else if (currentAction == "DELETE") {
      deleteLoop();
    }
    currentAction = "ACCESS"; // go back to access mode after admin task
    access();
  } else {
    lcd.clear();
    lcd.print("Wrong Password");
    input = ""; // clear the input after checking
  }
}

void fingerprintSensorSetup() {
  pinMode(2, INPUT);
  mySerial.begin(57000);
  attachInterrupt(0, fingerprintgo, FALLING);
  finger.begin(57000);
  pinMode(LED_BUILTIN, OUTPUT);
}

void relaySetup() {
  pinMode(relayPin, OUTPUT);         // Set relay pin as an output
  digitalWrite(relayPin, HIGH ); // Ensure relay starts off
}

void lcdSetup() {
  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();
}

void transistorSetup() {
  pinMode(transistorPin, OUTPUT); 
  digitalWrite(transistorPin, LOW);
  delay(1000);
  digitalWrite(transistorPin, HIGH);
}

int readnumberKeypad() {
  String numStr = "";
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Please enter ID:");
  lcd.setCursor(0, 1);
  while (true) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Thank you.");
        delay(2000);
        break;
      }
      numStr += key;
      lcd.setCursor(0, 1);
      lcd.print(numStr);
    }
  }
  return numStr.toInt();
}