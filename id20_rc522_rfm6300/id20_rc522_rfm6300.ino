// Set up the serial connection to the RFID reader module. In order to
// keep the Arduino TX and RX pins free for communication with a host,
// the sketch uses the SoftwareSerial library to implement serial
// communications on other pins.

//If using multiple software serial ports, only one can receive data at a time.
//Not all pins on the Mega and Mega 2560 support change interrupts, so only the following can be used for RX: 10, 11, 12, 13, 14, 15, 50, 51, 52, 53, A8 (62), A9 (63), A10 (64), A11 (65), A12 (66), A13 (67), A14 (68), A15 (69).

//Example sketch for RFID ID-20LA reader
//More info available at http://tutorial.cytron.com.my/

// Example sketch to read the ID from an Addicore 13.56MHz RFID tag
// as found in the RFID AddiKit found at:
// http://www.addicore.com/RFID-AddiKit-with-RC522-MIFARE-Module-RFID-Cards-p/126.htm

//wire ID20 tx on pin 3, Card Present on 7/13 any digital

/* adaption to MEGA:
    - IRQ not 2 or 3 as occupied. board does use pin 8 and that is OK with my design)
    - SPI somewhere else (not 10-13 and not 50-53 even if 53 free)
    - ... WIP
    - Serial1 on pins 19 (RX) and 18 (TX), Serial2 on pins 17 (RX) and 16 (TX), Serial3 on pins 15 (RX) and 14 (TX)
*/

/*
  NFC Communication with the Solutions Cubed, LLC BM019 and an Arduino Uno.  The BM019 is a module that carries ST Micro's CR95HF, a serial to NFC converter.
  Arduino          BM019
  IRQ: Pin 9       DIN: pin 2
  SS: pin 10       SS: pin 3
  MOSI: pin 11     MOSI: pin 5
  MISO: pin 12     MISO: pin4
  SCK: pin 13      SCK: pin 6

//#define BM019_TX p9
//#define BM019_RX p11

#define BM019_TX 18
#define BM019_RX 19

*/

//#include <MFRC522.h>
#include <AddicoreRFID.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <LiquidCrystal.h>

// WIEGAND
#include "RFID.h"

#define  uchar unsigned char
#define uint  unsigned int
#define MAX_LEN 16

// ID20 and similar
#define rxPin 11 //4
#define txPin 12 //6
#define cpPin 13 //7 ?

// Set up outputs
#define futureOutput 12
#define ledPin 13

// Specify how long the output should be held.
#define unlockSeconds 2

// INT_in : 2, INT_out : 6
/*
UART_RX/IRQ_OUT  2
UART_TX/IRQ_IN   8
Interface_Pin    9
SPI_CS_NFC      10
*/

#define MOSI    51 //51    // PB2, pin 21
#define MISO    50 //50    // PB3, pin 22
#define SCK     52 //52    // PB1, pin 20
#define SPI_MOSI    51 //51    // PB2, pin 21
#define SPI_MISO    50 //50    // PB3, pin 22
#define SPI_SCK     52 //52    // PB1, pin 20

//#define SPI_CS_NFC 10;
//#define SS      10 //53    // PB0, pin 19
#define SPI_SS      10 //53    // PB0, pin 19
//#define SPI_SS      9 //53    // PB0, pin 19 //cr95hf
//#define SPI_SS      49 //53    // PB0, pin 19

#define    uchar    unsigned char
#define    uint    unsigned int

const int SSPin = 10;  // Slave Select pin
const int IRQPin = 8;  // Sends wake-up pulse

const int _SSpin = 10; //53; // CS
const int _IRQpin = 8; //9;

//const int SSPin = 49;  // Slave Select pin //10
//const int IRQPin = 18;  // Sends wake-up pulse  // 2 conflicts with wiegand, move on 18, 19, 20, 21

byte TXBuffer[40];     // transmit buffer
byte RXBuffer[40];     // receive buffer

byte tagID[12];
boolean tagread = false;

int data1 = 0;

//522
byte NFCReady = 0;     // used to track NFC state
//4 bytes tag serial number, the first 5 bytes for the checksum byte
uchar serNumA[5];
uchar fifobytes;
uchar fifoValue;
const int chipSelectPin = 10;
const int NRSTPD = 5;

// The tag database consists of two parts. The first part is an array of tag values with each tag taking up 5 bytes. The second is a list of
// names with one name for each tag (ie: group of 5 bytes). You can expand or shrink this as you see fit. Tags 2 and 3 are only there for example.

char* allowedTags[] = {
  "[INSERT IDENTIFIER HERE]", // Tag 1
  "2900940E95", // Tag 2
  "ABC123DE45", // Tag 3
  "ABC123DE43", // Tag 4
  "ABC123DE41", // Tag 5
};

// List of names to associate with the matching tag IDs
char* tagName[] = {
  "[ADD YOUR NAME HERE]", // Tag 1
  "Mark Trussell", // Tag 2
  "NAME 3", // Tag 3
  "NAME 4", // Tag 4
  "NAME 5", // Tag 5
};

// Check the number of tags defined
int numberOfTags = sizeof(allowedTags) / sizeof(allowedTags[0]);

int incomingByte = 0; // To store incoming serial data

// Creates an RFID instance in Wiegand Mode DATA0 of the RFID Reader must be connected
// to Pin 2 of your Arduino (INT0 on most boards, INT1 on Leonardo)
// DATA1 of the RFID Reader must be connected to Pin 3 of your Arduino (INT1 on most boards, INT0 on Leonrado)
RFID wg(RFID_WIEGAND, W26BIT);

//LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
LiquidCrystal lcd(31, 33, 27, 29, 35, 37);

AddicoreRFID myRFID; // create AddicoreRFID object to control the RFID module  //MFRC522 myRFID

// Declares a struct to hold the data of the RFID tag
// Available fields:
//  * id (3 Bytes) - card code
//  * valid - validity
RFIDTag tag;
// WIEGAND-end

// Create a software serial object for the connection to the RFID module
SoftwareSerial rfid = SoftwareSerial(rxPin, txPin);

void setup() {

  //cr95hf
  pinMode(IRQPin, OUTPUT);
  digitalWrite(IRQPin, HIGH); // Wake up pulse
  pinMode(SSPin, OUTPUT);
  digitalWrite(SSPin, HIGH);

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  //SPI.setClockDivider(SPI_CLOCK_DIV16,32);
  SPI.setClockDivider(SPI_CLOCK_DIV32);

  pinMode(chipSelectPin, OUTPUT);             // Set digital pin 10 as OUTPUT to connect it to the RFID /ENABLE pin
  digitalWrite(chipSelectPin, LOW);         // Activate the RFID reader
  pinMode(NRSTPD, OUTPUT);                    // Set digital pin 10 , Not Reset and Power-down
  digitalWrite(NRSTPD, HIGH);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(futureOutput, OUTPUT);
  digitalWrite(futureOutput, LOW);

  //The CR95HF requires a wakeup pulse on its IRQ_IN pin before it will select UART or SPI mode.
  //The IRQ_IN pin is also the UART RX pin for DIN on the BM019 board.
  delay(10);                      // send a wake up pulse to put the BM019 into SPI mode
  digitalWrite(IRQPin, LOW);
  delayMicroseconds(10);
  digitalWrite(IRQPin, HIGH);
  delay(10);

  lcd.begin(16, 2);
  lcd.clear();
  delay(100);
  Serial.begin(9600);
  delay(100);
  rfid.begin(9600);
  //RFID reader SOUT pin connected to Serial RX pin at 9600bps
  delay(100);
  myRFID.AddicoreRFID_Init();

  Serial.println("INIT OK");
  lcd.setCursor(0, 0);
  lcd.print("    INIT OK     ");
}

/**
  Fire the relay to activate the strike plate for the configured
  number of seconds.
*/
void unlock() {
  digitalWrite(ledPin, HIGH);
  digitalWrite(futureOutput, HIGH);
  delay(unlockSeconds * 1000);
  digitalWrite(futureOutput, LOW);
  digitalWrite(ledPin, LOW);
}

/**
  Search for a specific tag in the database
*/
int findTag( char tagValue[10] ) {
  for (int thisCard = 0; thisCard < numberOfTags; thisCard++) {
    // Check if the tag value matches this row in the tag database
    if (strcmp(tagValue, allowedTags[thisCard]) == 0)
    {
      // The row in the database starts at 0, so add 1 to the result so
      // that the card ID starts from 1 instead (0 represents "no match")
      return (thisCard + 1);
    }
  }
  // If we don't find the tag return a tag ID of 0 to show there was no match
  return (0);
}

void loop() {
  int zz = 0;
  //WIEGAND
  if ( wg.available() ) {
    tag = wg.getTag();  // Retrieves the information of the tag
    Serial.print("CC = ");  // and prints that info on the serial port
    Serial.println(tag.id, HEX);
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(tag.id);
    //delay(1000);
    //Serial.print("The ID is ");
    //if (tag.valid) Serial.println("valid");
    //else Serial.println("invalid");
  }

  //RDM-6300
  int xx =0;
  //lcd.clear();
  lcd.setCursor(0,0);

  if (Serial.available() > 0) {
    data1 = Serial.read();
    Serial.println(data1, DEC);
    lcd.print(data1);
    xx=xx+2;
    if (xx>=16) {
      lcd.setCursor(xx,1);
    } else {
      lcd.setCursor(xx,0);
    }
  
    //delay(500);
    //lcd.clear();
  }
  delay(10);

  //ID20
  if (Serial.available() >= 13)   //Make sure all the frame is received
  {
    if (Serial.read() == 0x02)    //Check for the start byte = 0x02
    {
      tagread = true;              //New tag is read
      for (int index = 0; index < sizeof(tagID); index++) {
        byte val = Serial.read();

        //convert from ASCII value to value range from 0 to 9 and A to F
        if ( val >= '0' && val <= '9')
          val = val - '0';
        else if ( val >= 'A' && val <= 'F')
          val = 10 + val - 'A';

        tagID[index] = val;
      }
      //print_tag();                    //Display the tag ID
    } else {
      tagread = false;              //Discard and skip
    }
  }

  if (tagread == true)              //New tag is read
  {
    print_tag();                    //Display the tag ID
    clear_tag();                    //Clear the tag ID and ready for next tag
    tagread = false;
  }

  delay(10);

  //RC522
  uchar i, tmp, checksum1;
  uchar status;
  uchar str[MAX_LEN];
  uchar RC_size;
  uchar blockAddr;	//Selection operation block address 0 to 63
  String mynum = "";

  str[1] = 0x4400;
  //Find tags, return tag type
  status = myRFID.AddicoreRFID_Request(PICC_REQIDL, str);
  if (status == MI_OK) {
    Serial.println("RFID tag detected");
    Serial.print(str[0], BIN);
    Serial.print(" , ");
    Serial.print(str[1], BIN);
    Serial.println(" ");
  }

  //Anti-collision, return tag serial number 4 bytes
  status = myRFID.AddicoreRFID_Anticoll(str);
  if (status == MI_OK) {
    checksum1 = str[0] ^ str[1] ^ str[2] ^ str[3];
    Serial.println("The tag's number is  : ");
    //Serial.print(2);
    Serial.print(str[0]);
    Serial.print(" , ");
    Serial.print(str[1], BIN);
    Serial.print(" , ");
    Serial.print(str[2], BIN);
    Serial.print(" , ");
    Serial.print(str[3], BIN);
    Serial.print(" , ");
    Serial.print(str[4], BIN);
    Serial.print(" , ");
    Serial.println(checksum1, BIN);

    // Should really check all pairs, but for now we'll just use the first
    if (str[0] == 156)                     //You can change this to the first byte of your tag by finding the card's ID through the Serial Monitor
    {
      Serial.println("Hello Max !\n");
    } else if (str[0] == 244) {            //You can change this to the first byte of your tag by finding the card's ID through the Serial Monitor
      Serial.println("Goodbye Max !\n");
    }
    delay(10);
  } else {
    delay(10);
    //Serial.println();
  }
  
  myRFID.AddicoreRFID_Halt();		   //Command tag into hibernation

  //EXTRA LOOP

  //byte i = 0;
  byte r = 0;
  byte val = 0;
  byte checksum = 0;
  byte bytesRead = 0;
  byte tempByte = 0;
  byte tagBytes[6]; // "Unique" tags are only 5 bytes but we need an extra byte for the checksum
  char tagValue[10];

  // Read from the RFID module. Because this connection uses SoftwareSerial there is no equivalent to the Serial.available() function, so at this
  // point the program blocks while waiting for a value from the module

  if ((val = rfid.read()) == 2) { // Check for header
    bytesRead = 0;
    while (bytesRead < 12) { // Read 10 digit code + 2 digit checksum
      val = rfid.read();

      // Append the first 10 bytes (0 to 9) to the raw tag value
      if (bytesRead < 10) {
        tagValue[bytesRead] = val;
      }

      // Check if this is a header or stop byte before the 10 digit reading is complete
      if ((val == 0x0D) || (val == 0x0A) || (val == 0x03) || (val == 0x02)) {
        break; // Stop reading
      }

      // Ascii/Hex conversion:
      if ((val >= '0') && (val <= '9')) {
        val = val - '0';
      } else if ((val >= 'A') && (val <= 'F')) {
        val = 10 + val - 'A';
      }

      // Every two hex-digits, add a byte to the code:
      if (bytesRead & 1 == 1) {
        // Make space for this hex-digit by shifting the previous digit 4 bits to the left
        tagBytes[bytesRead >> 1] = (val | (tempByte << 4));

        if (bytesRead >> 1 != 5) { // If we're at the checksum byte,
          checksum ^= tagBytes[bytesRead >> 1]; // Calculate the checksum... (XOR)
        };
      } else {
        tempByte = val; // Store the first hex digit first
      };
      bytesRead++; // Ready to read next digit
    }

    if (bytesRead == 12) { // 12 digit read is complete
      tagValue[10] = '\0'; // Null-terminate the string

      Serial.print("Tag read: ");
      for (r = 0; r < 5; r++) {
        // Add a leading 0 to pad out values below 16
        if (tagBytes[r] < 16) {
          Serial.print("0");
        }
        Serial.print(tagBytes[i], HEX);
      }
      Serial.println();

      Serial.print("Checksum: ");
      Serial.print(tagBytes[5], HEX);
      Serial.println(tagBytes[5] == checksum ? " -- passed." : " -- error.");

      // Show the raw tag value
      Serial.print("VALUE: ");
      Serial.println(tagValue);

      // Search the tag database for this particular tag
      int tagId = findTag( tagValue );

      // Only fire the strike plate if this tag was found in the database
      if ( tagId > 0 ) {
        Serial.print("Authorized tag ID ");
        Serial.print(tagId);
        Serial.print(": unlocking for ");
        Serial.println(tagName[tagId - 1]); // Get the name for this tag from the database
        unlock(); // Fire the strike plate to open the lock
      } else {
        Serial.println("Tag not authorized");
      }
      Serial.println(); // Blank separator line in output
    }

    bytesRead = 0;
  }

  //CR95hf
  if (NFCReady == 0)
  {
    IDN_Command();  // reads the CR95HF ID
    //Mcu_Ver();
    delay(200);
    SetProtocol_Command(); // ISO 15693 settings
    //delay(1000);
  }
  else
  {
    Inventory_Command();
    //Mcu_Ver();
    delay(200);
  }
}

void print_tag()                    //Subroutine to display the tagID content
{
  //lcd.clear();
  lcd.setCursor(0, 0);
  int yy = 0;

  Serial.print("Tag ID: ");
  for (int index = 0; index < 10; index++) {
    Serial.print(tagID[index], HEX); //Display tagID
    lcd.setCursor(yy, 0);
    lcd.print(tagID[index]);
    yy = yy + 2;
  }
  Serial.print("\r\nChecksum: ");
  Serial.print(tagID[10], HEX);      //Display checksum
  Serial.println(tagID[11], HEX);
  delay(500);
}

void clear_tag()                    //Subroutine to clear the tagID content
{
  for (int index = 0; index < sizeof(tagID); index++)
  {
    tagID[index] = 0;
  }
}

/*
  int  CR95HFlib_Idn(char *StringReply)
  {
  unsigned char CmdArray[MAX_BUFFER_SIZE], RespArray[MAX_BUFFER_SIZE];

  if(!mgrObj.IsReadyforCommunication)
  {
    return STM32_connection_error;
  }
  else {
    CmdArray[0] = ID_SPI_SEND_DATA;
    CmdArray[1] = IDN;
    CmdArray[2] = 0x00;
    // Transmits the command
    mgrObj.WriteToDevice(CmdArray,3);
    // Recovers the reply
    mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);
    // Converts Hexa to Ascii
    Hex2Asc(StringReply, &RespArray[1], RespArray[2] + 2);
    return 0 ;
  }
  }
*/

/* IDN_Command identifies the CR95HF connected to the Arduino.
  This requires three steps.
  1. send command
  2. poll to see if CR95HF has data
  3. read the response

  If the correct response is received the serial monitor is used
  to display the CR95HF ID number and CRC code.  This rountine is
  not that useful in using the NFC functions, but is a good way to
  verify connections to the CR95HF.
*/

void IDN_Command() {
  byte i = 0;

  // step 1 send the command
  digitalWrite(SSPin, LOW);
  delayMicroseconds(10);
  SPI.transfer(0);  // SPI control byte to send command to CR95HF
  SPI.transfer(1);  // IDN command
  SPI.transfer(0);  // length of data that follows is 0
  delayMicroseconds(10);
  digitalWrite(SSPin, HIGH);
  delay(10);

  // step 2, poll for data ready
  // data is ready when a read byte
  // has bit 3 set (ex:  B'0000 1000')

  digitalWrite(SSPin, LOW);
  while (RXBuffer[0] != 8)
  {
    RXBuffer[0] = SPI.transfer(0x03);  // Write 3 until
    RXBuffer[0] = RXBuffer[0] & 0x08;  // bit 3 is set
  }
  digitalWrite(SSPin, HIGH);
  delay(1);

  // step 3, read the data
  digitalWrite(SSPin, LOW);
  delayMicroseconds(10);
  SPI.transfer(0x02);   // SPI control byte for read
  RXBuffer[0] = SPI.transfer(0);  // response code
  RXBuffer[1] = SPI.transfer(0);  // length of data
  for (i = 0; i < RXBuffer[1]; i++)
    RXBuffer[i + 2] = SPI.transfer(0); // data
  delayMicroseconds(10);
  digitalWrite(SSPin, HIGH);
  delay(1);

  if ((RXBuffer[0] == 0) & (RXBuffer[1] == 15))
  {
    Serial.println("IDN COMMAND-");  //
    Serial.print("RESPONSE CODE: ");
    Serial.print(RXBuffer[0]);
    Serial.print(" LENGTH: ");
    Serial.println(RXBuffer[1]);
    Serial.print("DEVICE ID: ");
    for (i = 2; i < (RXBuffer[1]); i++)
    {
      Serial.print(RXBuffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    Serial.print("ROM CRC: ");
    Serial.print(RXBuffer[RXBuffer[1]], HEX);
    Serial.print(RXBuffer[RXBuffer[1] + 1], HEX);
    Serial.println(" ");
  }
  else
    Serial.println("BAD RESPONSE TO IDN COMMAND!");

  Serial.println(" ");
}

/*
    if((RespCode != 0x80) && (RespCode != 0x90))
        if ( RespCode == 0x86 )
          strcpy(StringReply , "8600 : Communication error");
        else if ( RespCode == 0x87 )
          strcpy(StringReply , "8700 : Frame wait time out OR no tag");
        else if ( RespCode == 0x88 )
          strcpy(StringReply , "8800 : Invalid Start Of Frame");
        else if ( RespCode == 0x89 )
          strcpy(StringReply , "8900 : Receive buffer overflow (too many bytes received)");
        else if ( RespCode == 0x8A )
          strcpy(StringReply , "8A00: Framing error (start bit=0, stop bit=1)");
        else if ( RespCode == 0x8B )
          strcpy(StringReply , "8B00 : EGT time out (for Iso14443-B)");
        else if ( RespCode == 0x8C )
          strcpy(StringReply , "8C00 : Invalid length. Used in Felica, when field Length < 3");
        else if ( RespCode == 0x8D )
          strcpy(StringReply , "8D00 : CRC error (Used in Felica protocol)");
        else if ( RespCode == 0x8E )
          strcpy(StringReply , "8E00 : Reception lost without EOF received");
    if(RespCode != 0x00)
        if (RespCode == 0x82)
          strcpy(StringReply , "8200 : Invalid command length");
        else if ( RespCode == 0x83 )
          strcpy(StringReply , "8300 : Invalid protocol");
*/

/*

  int CR95HFlib_FieldOff(char* StringReply)
  {
  unsigned char CmdArray[MAX_BUFFER_SIZE], RespArray[MAX_BUFFER_SIZE];

  if(!mgrObj.IsReadyforCommunication)
  {
   return STM32_connection_error;
  }
  else{
   CmdArray[0] = ID_SPI_SEND_DATA;
   CmdArray[1] = SELECT;
   CmdArray[2] = 0x02;
   CmdArray[3] = 0x00;
   CmdArray[4] = 0x00;
   // Transmits the command
   mgrObj.WriteToDevice(CmdArray,5);
   // Recovers the reply
   mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);
   // Converts Hexa to Ascii
   Hex2Asc(StringReply, &RespArray[1], RespArray[2] + 2);
   return 0 ;
  }
  }
*/

/* SetProtocol_Command programs the CR95HF for
  ISO/IEC 15693 operation.

  This requires three steps.
  1. send command
  2. poll to see if CR95HF has data
  3. read the response

  If the correct response is received the serial monitor is used
  to display successful programming.
*/

void SetProtocol_Command()
{
  byte i = 0;

  // step 1 send the command
  digitalWrite(SSPin, LOW);
  delayMicroseconds(10);

  SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
  SPI.transfer(0x02);  // Set protocol command
  SPI.transfer(0x02);  // length of data to follow
  SPI.transfer(0x01);  // code for ISO/IEC 15693
  SPI.transfer(0x0D);  // Wait for SOF, 10% modulation, append CRC

  delayMicroseconds(10);
  digitalWrite(SSPin, HIGH);
  delay(1);

  // step 2, poll for data ready

  digitalWrite(SSPin, LOW);
  while (RXBuffer[0] != 8)
  {
    RXBuffer[0] = SPI.transfer(0x03);  // Write 3 until
    RXBuffer[0] = RXBuffer[0] & 0x08;  // bit 3 is set
  }
  digitalWrite(SSPin, HIGH);
  delay(1);

  // step 3, read the data
  digitalWrite(SSPin, LOW);
  delayMicroseconds(10);
  SPI.transfer(0x02);   // SPI control byte for read
  RXBuffer[0] = SPI.transfer(0);  // response code
  RXBuffer[1] = SPI.transfer(0);  // length of data
  delayMicroseconds(10);
  digitalWrite(SSPin, HIGH);
  delay(1);

  if ((RXBuffer[0] == 0) & (RXBuffer[1] == 0))
  {
    Serial.println("PROTOCOL SET-");  //
    NFCReady = 1; // NFC is ready
  }
  else
  {
    Serial.println("BAD RESPONSE TO SET PROTOCOL");
    NFCReady = 0; // NFC not ready
  }

  Serial.println(" ");
}

/* Inventory_Command chekcs to see if an RF
  tag is in range of the BM019.

  This requires three steps.
  1. send command
  2. poll to see if CR95HF has data
  3. read the response

  If the correct response is received the serial monitor is used
  to display the the RF tag's universal ID.
*/

void Inventory_Command()
{
  byte i = 0;

  // step 1 send the command
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
  SPI.transfer(0x04);  // Send Receive CR95HF command
  SPI.transfer(0x03);  // length of data that follows is 0
  SPI.transfer(0x26);  // request Flags byte
  SPI.transfer(0x01);  // Inventory Command for ISO/IEC 15693
  SPI.transfer(0x00);  // mask length for inventory command
  delayMicroseconds(10);
  digitalWrite(SSPin, HIGH);
  delay(1);

  // step 2, poll for data ready
  // data is ready when a read byte
  // has bit 3 set (ex:  B'0000 1000')

  digitalWrite(SSPin, LOW);
  while (RXBuffer[0] != 8)
  {
    RXBuffer[0] = SPI.transfer(0x03);  // Write 3 until
    RXBuffer[0] = RXBuffer[0] & 0x08;  // bit 3 is set
  }
  digitalWrite(SSPin, HIGH);
  delay(1);

  // step 3, read the data
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x02);   // SPI control byte for read
  RXBuffer[0] = SPI.transfer(0);  // response code
  RXBuffer[1] = SPI.transfer(0);  // length of data
  for (i = 0; i < RXBuffer[1]; i++)
    RXBuffer[i + 2] = SPI.transfer(0); // data
  digitalWrite(SSPin, HIGH);
  delay(1);

  if (RXBuffer[0] == 128) {
    //Serial.println("TAG DETECTED");
    Serial.print("CR95HF UID: ");
    for (i = 11; i >= 4; i--)
    {
      Serial.print(RXBuffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
  }
  /* else {
    //Serial.print("NO TAG IN RANGE - ");
    //Serial.print("RESPONSE CODE: ");
    //Serial.println(RXBuffer[0],HEX);

    for(i=11;i>=4;i--)
    {
      Serial.print(RXBuffer[i],HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    }

    }
  */

}

void Mcu_Ver() {
  byte i = 0;

  // step 1 send the command
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
  SPI.transfer(0x04);  // Send Receive CR95HF command
  SPI.transfer(0x02);  // length of data that follows is 0
  SPI.transfer(0x26);  // request Flags byte
  SPI.transfer(0xBC);  // MCU VERSION COMMAND
  SPI.transfer(0x00);  // mask length for inventory command
  delayMicroseconds(10);
  digitalWrite(SSPin, HIGH);
  delay(1);

  // step 2, poll for data ready
  // data is ready when a read byte
  // has bit 3 set (ex:  B'0000 1000')

  digitalWrite(SSPin, LOW);
  while (RXBuffer[0] != 8)
  {
    RXBuffer[0] = SPI.transfer(0x03);  // Write 3 until
    RXBuffer[0] = RXBuffer[0] & 0x08;  // bit 3 is set
  }
  digitalWrite(SSPin, HIGH);
  delay(1);

  // step 3, read the data
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x02);   // SPI control byte for read
  RXBuffer[0] = SPI.transfer(0);  // response code
  RXBuffer[1] = SPI.transfer(0);  // length of data
  for (i = 0; i < RXBuffer[1]; i++)
    RXBuffer[i + 2] = SPI.transfer(0); // data
  delayMicroseconds(10);
  digitalWrite(SSPin, HIGH);
  delay(1);

  //  if ((RXBuffer[0] == 0) & (RXBuffer[1] == 15))

  Serial.print("RESPONSE CODE: ");
  Serial.print(RXBuffer[0]);
  Serial.print(" LENGTH: ");
  Serial.println(RXBuffer[1]);
  Serial.print("MCU VER: ");
  for (i = 2; i < (RXBuffer[1]); i++)
  {
    Serial.print(RXBuffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("MCU VER: ");
  Serial.print(RXBuffer[RXBuffer[1]], HEX);
  Serial.print(RXBuffer[RXBuffer[1] + 1], HEX);
  Serial.println(" ");

  /*
    if (RXBuffer[0] == 128) {
      Serial.println("VER CMD OK");
      Serial.print("VER: ");
      for(i=11;i>=4;i--)
      {
        Serial.print(RXBuffer[i],HEX);
        Serial.print(" ");
      }
    }
  */

}

