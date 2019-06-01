
//Library To read and write PICC's UIDs from/to EEPROM
#include <SPI.h>
#include <EEPROM.h>
#include <require_cpp11.h>
#include <MFRC522.h>
#include <deprecated.h>
#include <LiquidCrystal.h>
#include <Servo.h>

  

//Variables globales
byte storedCard[4];    // Stores an ID read from EEPROM
byte readCard[4];      // Stores scanned ID read from RFID Module
// Store Ref iD to compare with read ID
byte MasterCardID[4] = {0x4B, 0x70, 0xBE, 0x49};   // here use the correct ID value

// peut etre utiliser un char StrReadCarID[32] puis array_to_string(array, 4, StrReadCarID);
char StrReadCardID[32]="";
char strMasterCardID[32]="";
uint8_t successRead; // Variable integer to keep if we have Successful Read from Reader

// Pour le LCD 1602
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Variables globales de gestion du moteur Servo
Servo myservo;
int pos = 0; 



// Affectation des broches
#define RST_PIN 9
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
    // Initialisation du Module RFID
    myservo.attach(8); 
    Serial.begin(9600);
    while(!Serial);
    SPI.begin();
    mfrc522.PCD_Init();
    mfrc522.PCD_DumpVersionToSerial(); 
    
    // Affichage des données de la bibliothèque
    Serial.println(F("Scan PICC to see UID, type, and data blocks..."));
}

void loop() {
    do {
        successRead = getID(); 
    }
    while (!successRead);

    /* Code déplacé dans la fonction getID()
    // Attente d'une carte RFID
    if( ! mfrc522.PICC_IsNewCardPresent()) {return;}
    // Récupération des informations de la carte RFID
    if( ! mfrc522.PICC_ReadCardSerial()) {return;}
    // Affichagedes informations de la carte RFID
    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
    */
}

// FONCTION DE RESSOURCE POUR UTILISATION DANS LA FONCTION LOOP
///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  //Pour comparer les valeur des 2 ID on peut utilise String(char[32])
  array_to_string(readCard, 4, StrReadCardID);
  array_to_string(MasterCardID, 4, strMasterCardID);

  if (String(strMasterCardID) == String(StrReadCardID)) {
      Serial.println("AUTHORIZED");
      // ici mettre le code pour écrire sur le LCD
      lcd.begin(16, 2);
      // Print a message to the LCD.
      lcd.setCursor(0, 1);
      lcd.println("AUTHORIZED !");
      for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        myservo.write(pos);              // tell servo to go to position in variable 'pos'
        delay(15);                       // waits 15ms for the servo to reach the position
      }
      delay(5000);
      for (pos = 90; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
        myservo.write(pos);              // tell servo to go to position in variable 'pos'
        delay(15);                       // waits 15ms for the servo to reach the position
      }
  } else {
      Serial.println("NOT AUTHORIZED");
      // ici mettre le code pour écrire sur le LCD
      lcd.begin(16, 2);
      // Print a message to the LCD.
      lcd.setCursor(0, 1);
      lcd.println("NOT AUTHORIZED !");
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

//////////////////////////////////Function to convert bytes Array ID to Char Buffer 
void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}

//////////////////////////////////////// Function to compare readCard and MasterCardID ///////
/*int CheckToken (uint8_t CTuid[7])
{
  int index = 0;
  const int tokens = 10;
  while (index < tokens)
  {
    if (
      (memcmp (CTuid, database[index].token1, 7) == 0)
      or
      (memcmp (CTuid, database[index].token2, 7) == 0)
      or
      (memcmp (CTuid, database[index].token3, 7) == 0)
      )
      break;
    index++;
  }
  
  Serial.print(F("Token "));
  if (index < tokens)
  {
      Serial.print(index);
  }
  else 
  {
      Serial.print(F("not"));
      index = -1;
  }
  Serial.println(F(" found in EEPROM"));

  return index;
}*/
