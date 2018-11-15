/*
NFC Communication with the Solutions Cubed, LLC BM019  and an Arduino Uno.  The BM019 is a module that carries ST Micro's CR95HF, a serial to NFC converter.
 Arduino          BM019
 IRQ: Pin 9       DIN: pin 2
 SS: pin 10       SS: pin 3
 MOSI: pin 11     MOSI: pin 5 
 MISO: pin 12     MISO: pin4
 SCK: pin 13      SCK: pin 6
*/

#include <SPI.h>

#define MOSI    51 //51    // PB2, pin 21
#define MISO    50 //50    // PB3, pin 22
#define SCK     52 //52    // PB1, pin 20
#define SPI_MOSI    51 //51    // PB2, pin 21
#define SPI_MISO    50 //50    // PB3, pin 22
#define SPI_SCK     52 //52    // PB1, pin 20

#define SPI_CS_NFC  10
#define SS          10 //53    // PB0, pin 19
#define SPI_SS      10 //53    // PB0, pin 19

const int SSPin = 10;  // Slave Select pin
const int IRQPin = 8;  // Sends wake-up pulse

byte TXBuffer[40];    // transmit buffer
byte RXBuffer[40];    // receive buffer
byte NFCReady = 0;  // used to track NFC state
byte Memory_Block = 0; // keep track of memory we write to
byte Data = 0; // keep track of memory we read from     

void setup() {
    pinMode(IRQPin, OUTPUT);
    digitalWrite(IRQPin, HIGH); // Wake up pulse
    pinMode(SSPin, OUTPUT);
    digitalWrite(SSPin, HIGH);

    Serial.begin(9600);
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV16);
    //    SPI.setClockDivider(SPI_CLOCK_DIV32);
 // The CR95HF requires a wakeup pulse on its IRQ_IN pin
 // before it will select UART or SPI mode.  The IRQ_IN pin
 // is also the UART RX pin for DIN on the BM019 board.
 
    delay(10);                      // send a wake up
    digitalWrite(IRQPin, LOW);      // pulse to put the 
    delayMicroseconds(100);         // BM019 into SPI
    digitalWrite(IRQPin, HIGH);     // mode 
    delay(10);
}

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
  SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
  SPI.transfer(0x02);  // Set protocol command
  SPI.transfer(0x02);  // length of data to follow
  SPI.transfer(0x01);  // code for ISO/IEC 15693
  SPI.transfer(0x0D);  // Wait for SOF, 10% modulation, append CRC
  digitalWrite(SSPin, HIGH);
  delay(1);
 
// step 2, poll for data ready

  digitalWrite(SSPin, LOW);
  while(RXBuffer[0] != 8)
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
  digitalWrite(SSPin, HIGH);

  if ((RXBuffer[0] == 0) & (RXBuffer[1] == 0))  // is response code good?
    {
    Serial.println("Protocol Set Command OK");
    NFCReady = 1; // NFC is ready
    }
  else
    {
    Serial.println("Protocol Set Command FAIL");
    NFCReady = 0; // NFC not ready
    }
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
  digitalWrite(SSPin, HIGH);
  delay(1);
 
// step 2, poll for data ready
// data is ready when a read byte
// has bit 3 set (ex:  B'0000 1000')

  digitalWrite(SSPin, LOW);
  while(RXBuffer[0] != 8)
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
  for (i=0;i<RXBuffer[1];i++)      
      RXBuffer[i+2]=SPI.transfer(0);  // data
  digitalWrite(SSPin, HIGH);
  delay(1);

  if (RXBuffer[0] == 128)  // is response code good?
    {
    Serial.println("Inventory Command OK");
    NFCReady = 2;
    }
  else
    {
    Serial.println("Inventory Command FAIL");
    NFCReady = 0;
    }
 }
 
/* Write Memory writes data to a block of memory.  
This code assumes the RF tag has less than 256 blocks
of memory and that the block size is 4 bytes.  The block 
written to increments with each pass and if it exceeds
20 it starts over. Data is also changed with each write.

This requires three steps.
1. send command
2. poll to see if CR95HF has data
3. read the response
*/

void Write_Memory()
 {
 byte i = 0;
    
  Data = Data +4;
  Memory_Block = Memory_Block +1;
  if (Memory_Block > 20)
    Memory_Block = 0;
  
// step 1 send the command
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
  SPI.transfer(0x04);  // Send Receive CR95HF command
  SPI.transfer(0x07);  // length of data that follows 
//  SPI.transfer(0x08);  //Use this length if more than 256 mem blocks 
  SPI.transfer(0x02);  // request Flags byte
//  SPI.transfer(0x0A);  // request Flags byte if more than 256 memory blocks
  SPI.transfer(0x21);  // Write Single Block command for ISO/IEC 15693
  SPI.transfer(Memory_Block);  // Memory block address
//  SPI.transfer(0x00);  // MSB of mem. address greater than 256 blocks
  SPI.transfer(Data);  // first byte block of memory block
  SPI.transfer(Data+1);  // second byte block of memory block
  SPI.transfer(Data+2);  // third byte block of memory block
  SPI.transfer(Data+3);  // third byte block of memory block
  digitalWrite(SSPin, HIGH);
  delay(1);
 
// step 2, poll for data ready
// data is ready when a read byte
// has bit 3 set (ex:  B'0000 1000')

  digitalWrite(SSPin, LOW);
  while(RXBuffer[0] != 8)
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
  for (i=0;i<RXBuffer[1];i++)      
      RXBuffer[i+2]=SPI.transfer(0);  // data
  digitalWrite(SSPin, HIGH);
  delay(1);

  if (RXBuffer[0] == 128)  // is response code good?
    {
    Serial.print("Write Memory Block Command OK - Block ");
    Serial.print(Memory_Block);
    Serial.print(":  ");
    Serial.print(Data);
    Serial.print(" ");
    Serial.print(Data+1);
    Serial.print(" ");
    Serial.print(Data+2);
    Serial.print(" ");
    Serial.print(Data+3);
    Serial.println(" ");
    NFCReady = 2;
    }
  else
    {
    Serial.print("Write Memory Block Command FAIL");
    NFCReady = 0;
    }
 }

/* Read Memory reads data from a block of memory.  
This code assumes the RF tag has less than 256 blocks
of memory and that the block size is 4 bytes.  Data
read from is displayed via the serial monitor.

This requires three steps.
1. send command
2. poll to see if CR95HF has data
3. read the response
*/

void Read_Memory()
 {
 byte i = 0;
   
// step 1 send the command
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
  SPI.transfer(0x04);  // Send Receive CR95HF command
  SPI.transfer(0x03);  // length of data that follows
 // SPI.transfer(0x04);  // length of data if mem > 256
  SPI.transfer(0x02);  // request Flags byte
 // SPI.transfer(0x0A);  // request Flags byte if mem > 256
  SPI.transfer(0x20);  // Read Single Block command for ISO/IEC 15693
  SPI.transfer(Memory_Block);  // memory block address
//  SPI.transfer(0x00);  // MSB of memory block address if mem > 256
  digitalWrite(SSPin, HIGH);
  delay(1);
 
// step 2, poll for data ready
// data is ready when a read byte
// has bit 3 set (ex:  B'0000 1000')

  digitalWrite(SSPin, LOW);
  while(RXBuffer[0] != 8)
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
  for (i=0;i<RXBuffer[1];i++)      
      RXBuffer[i+2]=SPI.transfer(0);  // data
  digitalWrite(SSPin, HIGH);
  delay(1);

  if (RXBuffer[0] == 128) // is response code good?
    {
    Serial.print("Read Memory Block Command OK - Block ");
    Serial.print(Memory_Block);
    Serial.print(":  ");
    Serial.print(RXBuffer[3]);
    Serial.print(" ");
    Serial.print(RXBuffer[4]);
    Serial.print(" ");
    Serial.print(RXBuffer[5]);
    Serial.print(" ");
    Serial.print(RXBuffer[6]);
    Serial.println(" ");
    NFCReady = 2;
    }
  else
    {
    Serial.print("Read Memory Block Command FAIL");
    NFCReady = 0;
    }
 }
 
void loop() {
  
  if(NFCReady == 0)
    {
    delay(50);  
    SetProtocol_Command(); // ISO 15693 settings
    }
  else if (NFCReady == 1)
    {
    delay(50);  
    Inventory_Command();
    }
  else
    {
    Write_Memory();
    delay(50);  
    Read_Memory();
    }
    Serial.println(" ");  
    delay(50);  
}

