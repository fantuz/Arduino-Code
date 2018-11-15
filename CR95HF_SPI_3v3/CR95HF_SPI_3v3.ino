/*
Near Field Communication, or NFC, with the Arduino can be accomplished using our BM019 serial to NFC converter module.
NFC is used for short range exchanges of data.  It can be used to read and write to smart cards and to interface with parasitically powered EEPROM. 
One interesting application of NFC is the post production programming of variables into  electronic assemblies. 
You could, for example, program a set of variables into a motion control module by waving a NFC master unit over an EEPROM located on an unpowered board, and adjust the EEPROM contents.
The BM019 module carries ST Micro’s CR95HF serial to NFC converter as well as other circuitry including a tuned antenna. 
The CR95HF can operate with a serial UART (8N1 57.6KBPS) or with SPI (serial peripheral interface).
When I use the Arduino for serial data applications I like to use the software serial functions.
This leaves the hardware serial port open for both programming and diagnostics.  Unfortunately, the software serial functions don’t fare too well when pushed beyond 9600BPS (baud rate).
The BM019 works at too high a speed, so I opted to use interface the Arduino using the BM019 SPI port.
One issue I had to address was the voltage difference between the Arduino and the BM019.  The Arduino operates with 5V logic and the BM019 with 3.3V logic.
Therefore all of the inputs to the BM019 need to be reduced from 5V to 3.3V.  This is easily done with resistor voltage dividers.
I also slowed the SPI clock since I had added resistors to the interface.  If you wanted to run the communication faster you could use a voltage level translator.
Here’s the schematic for the connections.  It is pretty simple (click image for clearer view).

The CR95HF has a quirky power-up process.  After power-up you need to send it a wake-up pulse.  This pulse tells it to which serial interface to use (SPI or UART). 
The pulse must be sent on the DIN pin of the UART interface.  If you’re using the UART you just send a 0x00 after power up.  That’s easy.
But if you are using SPI you have to allocate another connection to the BM019 to accomplish the wake-up pulse.

To operate in SPI mode you tie SS_0 to 3.3V and provides a short low pulse on the DIN pin (note: the image below shows powering up in UART mode, but provides useful timing requirements). clip_image002
Operating the Arduino in SPI mode requires that we setup the i/o’s correctly, enable the SPI functions, and then fire off the wake-up pulse.

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
const int IRQPin = 8;  // Sends wake-up pulse //2

byte TXBuffer[40];    // transmit buffer
byte RXBuffer[40];    // receive buffer
byte NFCReady = 0;  // used to track NFC state

void setup() {
    pinMode(IRQPin, OUTPUT);
    digitalWrite(IRQPin, HIGH); // Wake up pulse
    pinMode(SSPin, OUTPUT);
    digitalWrite(SSPin, HIGH);
    delay(100);
    
    //Serial.begin(9600);
    //Serial.write("Ready!");
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV32);

 // The CR95HF requires a wakeup pulse on its IRQ_IN pin
 // before it will select UART or SPI mode.  The IRQ_IN pin
 // is also the UART RX pin for DIN on the BM019 board.

    delay(10);                      // send a wake up
    digitalWrite(IRQPin, LOW);      // pulse to put the 
    delayMicroseconds(100);         // BM019 into SPI
    digitalWrite(IRQPin, HIGH);     // mode 
    //delay(10);
    delay(100);
    
    Serial.begin(9600);
    Serial.write("Ready!");
}

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
void IDN_Command()
 {
 byte i = 0;

// step 1 send the command
  digitalWrite(SSPin, LOW);
  SPI.transfer(0);  // SPI control byte to send command to CR95HF
  SPI.transfer(1);  // IDN command
  SPI.transfer(0);  // length of data that follows is 0
  digitalWrite(SSPin, HIGH);
  delay(10);

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
  delay(10);

// step 3, read the data
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x02);   // SPI control byte for read         
  RXBuffer[0] = SPI.transfer(0);  // response code
  RXBuffer[1] = SPI.transfer(0);  // length of data
  for (i=0;i<RXBuffer[1];i++)      
      RXBuffer[i+2]=SPI.transfer(0);  // data
  digitalWrite(SSPin, HIGH);
  delay(10);

  if ((RXBuffer[0] == 0) & (RXBuffer[1] == 15)) {  
    Serial.println("IDN COMMAND-");  //
    Serial.print("RESPONSE CODE: ");
    Serial.print(RXBuffer[0]);
    Serial.print(" LENGTH: ");
    Serial.println(RXBuffer[1]);
    Serial.print("DEVICE ID: ");
    for(i=2;i<(RXBuffer[1]);i++)
    {
      Serial.print(RXBuffer[i],HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    Serial.print("ROM CRC: ");
    Serial.print(RXBuffer[RXBuffer[1]],HEX);
    Serial.print(RXBuffer[RXBuffer[1]+1],HEX);
    Serial.println(" ");
  } else {
    Serial.println("BAD RESPONSE TO IDN COMMAND!");
  }

  Serial.println(" ");
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
  delay(10);

// step 2, poll for data ready

  digitalWrite(SSPin, LOW);
  while(RXBuffer[0] != 8)
    {
    RXBuffer[0] = SPI.transfer(0x03);  // Write 3 until
    RXBuffer[0] = RXBuffer[0] & 0x08;  // bit 3 is set
    }
  digitalWrite(SSPin, HIGH);
  delay(10);

// step 3, read the data
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x02);   // SPI control byte for read         
  RXBuffer[0] = SPI.transfer(0);  // response code
  RXBuffer[1] = SPI.transfer(0);  // length of data
  digitalWrite(SSPin, HIGH);
  delay(10);

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
  digitalWrite(SSPin, HIGH);
  delay(10);

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
  delay(10);

// step 3, read the data
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x02);   // SPI control byte for read         
  RXBuffer[0] = SPI.transfer(0);  // response code
  RXBuffer[1] = SPI.transfer(0);  // length of data
  for (i=0;i<RXBuffer[1];i++)      
      RXBuffer[i+2]=SPI.transfer(0);  // data
  digitalWrite(SSPin, HIGH);
  delay(10);

  if (RXBuffer[0] == 128)
  {  
    Serial.println("TAG DETECTED");
    Serial.print("UID: ");
    for(i=8;i>=1;i--)
    //for(i=31;i>=4;i--)
    {
      Serial.print(RXBuffer[i],HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
  }
  else
    {
    Serial.print("NO TAG IN RANGE - ");
    Serial.print("RESPONSE CODE: ");
    Serial.println(RXBuffer[0],HEX);
    }
  Serial.println(" ");
}

void loop() {

  if(NFCReady == 0)
  {
    IDN_Command();  // reads the CR95HF ID
    delay(1000);
    SetProtocol_Command(); // ISO 15693 settings
    delay(1000);
  } else {
    Inventory_Command();
    delay(1000);    
  }

}

