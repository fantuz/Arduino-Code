//#include <Wiegand.h>

//#include <RFID.h>
#include <SoftwareSerial.h> 
#include <SeeedRFIDLib.h>

#define DEBUG 1;
#define HAVE_RFID_WIEGAND 1;
#define HAVE_RFID_UART 0;

// SoftwareSerial must be included because the library depends on it
// If you use the Library in Wiegand Mode, then the compiler
// will optimize the SoftwareSerial away

/**** 
 * Struct for storing an RFID tag
 */
/*
  struct RFIDTag {
  int mfr;         // Manufacturer (?) Code (2 bytes), only useful in UART Mode
  long id;         // Tag ID (3 bytes)
  byte chk;        // Checksum (1 byte), only useful in UART Mode
  boolean valid;   // Validity of the Tag, based on the Checksum (UART Mode) or the parity bits (Wiegand Mode)
  char raw[13];    // The whole tag as a raw string, only useful in UART Mode
};
*/

//WIEGAND RFID; //(WIEGAND_26BIT); 
//RFID RFID; //(RFID_WIEGAND);

//RFID rfid(RFID_WIEGAND, W35BIT);
// W26BIT or W35BIT

SeeedRFIDLib RFID(); 
//SeeedRFIDLib RFID(RFID_WIEGAND); 
//WIEGAND_26BIT

RFIDTag tag;

void setup() {
  Serial.begin(9600);
  //RFID.resetWiegand();
  //rfid.initWiegand();
  //RFID.resetWiegand();
  //rfid.RFIDMode(RFID_WIEGAND);
  while ( !Serial );
}

void loop() {
    if(SeeedRFIDLib.isIdAvailableWiegand()) {
//    if(rfid.available()) {
        tag = rfid.getTag();  // Retrieves the information of the tag
        //SeeedRFIDLib.readId();
        //Serial.print("CC = ");  // and prints that info on the serial port
        //for(int i = 0; i < sizeof(tag.raw); i++)
        Serial.println(tag.id, HEX);
        for(int i = 0; i < 13; i++)
          {
            Serial.print(tag.raw[i]);
          }
        //if (!tag.valid) Serial.println("The ID is invalid");
      /*
        rfid.getTag();
        // In Wiegand Mode, we only get the card code
        Serial.print("CC = ");
        Serial.println(tag.id); 
      */  
        //rfid.refreshWiegand();
        //rfid.checkParity35();
      /*
    if(RFID.isIdAvailable()) {
        tag = RFID.readId();
        // In Wiegand Mode, we only get the card code
        Serial.print("CC = ");
        Serial.println(tag.id); 
    }
      */        
    }
}
