#include <Adafruit_Fingerprint.h>
SoftwareSerial mySerial(3, 4);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
bool go = false;

// RELAY LOGIC
const int relayPin = 8;       // Pin connected to the relay IN pin
const int transistorPin = 13;


#include <LiquidCrystal_I2C.h>

#include <Wire.h>
//initialize the liquid crystal library
//the first parameter is  the I2C address
//the second parameter is how many rows are on your screen
//the  third parameter is how many columns are on your screen
LiquidCrystal_I2C lcd(0x27,  16, 2);


void setup() {

  Serial.begin(9600);
  
  fingerprintSensorSetup();

  relaySetup();

  lcdSetup();

  pinMode(transistorPin, OUTPUT); 
  digitalWrite(transistorPin, LOW);
  delay(1000);
  digitalWrite(transistorPin, HIGH);

  Serial.println("Setup complete.");
}

void loop() {
  if (!go){
    Serial.println("Will go");
    lcd.backlight();
    getFingerprintID();
    lcd.noBacklight();
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
  // initialize lcd screen
  lcd.init();
  // turn on the backlight
  lcd.backlight();
}
