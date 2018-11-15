/*
The BM019 is a module that carries ST Micro's CR95HF, a serial to NFC converter.

BM019  Arduino Mega2560
MOSI---------D51
MISO---------D50     
SCK----------D52
 
DIN1---------D48   SS1----------D49
DIN2---------D46   SS2----------D47
DIN3---------D44   SS3----------D45
DIN4---------D42   SS4----------D43
*/

#include <SPI.h>

const int DINPin1 = 48;  // Sends wake-up pulse
const int SSPin1 = 49;  // Slave Select pin

const int DINPin2 = 46;  // Sends wake-up pulse
const int SSPin2 = 47;  // Slave Select pin

const int DINPin3 = 44;  // Sends wake-up pulse
const int SSPin3 = 45;  // Slave Select pin

const int DINPin4 = 42;  // Sends wake-up pulse
const int SSPin4 = 43;  // Slave Select pin

byte TXBuffer[40];    // transmit buffer
byte RXBuffer[40];    // receive buffer

byte NFCReady1 = 0;  // used to track NFC state
byte NFCReady2 = 0;  // used to track NFC state
byte NFCReady3 = 0;
byte NFCReady4 = 0;

//byte Memory_Block = 0; // keep track of memory we write to
byte siteBlock  = 4;//  keep track of memory we write to 
byte Data = 0; // keep track of memory we read from     

void setup() {
    /*pinMode(IRQPin, OUTPUT);
    digitalWrite(IRQPin, HIGH); // Wake up pulse
    pinMode(SSPin, OUTPUT);
    digitalWrite(SSPin, HIGH);
    */
    pinMode(53,OUTPUT);
    digitalWrite(53,HIGH);//disable device. 
    pinMode(DINPin1,OUTPUT);
    digitalWrite(DINPin1,HIGH);// wake up pulse
    pinMode(SSPin1,OUTPUT);
    digitalWrite(SSPin1,HIGH);
    
    pinMode(DINPin2,OUTPUT);
    digitalWrite(DINPin2,HIGH);// wake up pulse
    pinMode(SSPin2,OUTPUT);
    digitalWrite(SSPin2,HIGH);   

    pinMode(DINPin3,OUTPUT);
    digitalWrite(DINPin3,HIGH);// wake up pulse
    pinMode(SSPin3,OUTPUT);
    digitalWrite(SSPin3,HIGH);  

    pinMode(DINPin4,OUTPUT);
    digitalWrite(DINPin4,HIGH);// wake up pulse
    pinMode(SSPin4,OUTPUT);
    digitalWrite(SSPin4,HIGH);  

    Serial.begin(9600);
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV32);
 
 // The CR95HF requires a wakeup pulse on its IRQ_IN pin before it will select UART or SPI mode.  The IRQ_IN pin is also the UART RX pin for DIN on the BM019 board.
 
/*  
 *  delay(10);                      // send a wake up
    digitalWrite(IRQPin, LOW);      // pulse to put the 
    delayMicroseconds(100);         // BM019 into SPI
    digitalWrite(IRQPin, HIGH);     // mode 
    delay(10);
 */

 delay(10);
 digitalWrite(DINPin1,LOW); //send a wake up
 digitalWrite(DINPin2,LOW); //send a wake up
 digitalWrite(DINPin3,LOW); //send a wake up
 digitalWrite(DINPin4,LOW);
 delayMicroseconds(100); //pulse to put the 
 digitalWrite(DINPin1,HIGH);//BM019 into SPI mode
 digitalWrite(DINPin2,HIGH);//BM019 into SPI mode
 digitalWrite(DINPin3,HIGH);//BM019 into SPI mode
 digitalWrite(DINPin4,HIGH);
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
byte SetProtocol_Command(const int SSPin)
 {
 byte i = 0;
 byte NFCReady;
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
  delay(100);
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
    Serial.print("Protocol Set Command ");
    Serial.print(SSPin);
    Serial.println(" OK");
    NFCReady = 1; // NFC is ready
    }
  else
    {
    Serial.print("Protocol Set Command ");
    Serial.print(SSPin);
    Serial.println(" FAILED");
    NFCReady = 0; // NFC not ready
    }
    return NFCReady;
}

/* Read Memory reads data from siteBlock.  
This code assumes the RF tag has less than 256 blocks
of memory and that the block size is 4 bytes.  Data
read from is displayed via the serial monitor.
*/
byte Read_Memory(const int SSPin) {
 byte i = 0;
 byte NFCReady;
// step 1 send the command
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
  SPI.transfer(0x04);  // Send Receive CR95HF command
  SPI.transfer(0x03);  // length of data that follows
 // SPI.transfer(0x04);  // length of data if mem > 256
  SPI.transfer(0x02);  // request Flags byte
 // SPI.transfer(0x0A);  // request Flags byte if mem > 256
  SPI.transfer(0x20);  // Read Single Block command for ISO/IEC 15693
  //SPI.transfer(0x23);//read multiple block command
  SPI.transfer(siteBlock);  // memory block address
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
  for (i=0;i<RXBuffer[1];i++){      
      RXBuffer[i+2]=SPI.transfer(0);
  }// data
  digitalWrite(SSPin, HIGH);
  delay(1);

  if (RXBuffer[0] == 128) // is response code good?
    {
    Serial.print("Read Memory Command OK");
    Serial.print('\n');
    Serial.print("RXBuffer is: ");
    Serial.write(RXBuffer[3]);
    Serial.print("Site number: ");
    switch (RXBuffer[3])
    {
      case 1:
      Serial.print("A"); break;
      case 2:
      Serial.print("B");break;
      case 3:
      Serial.print("C");break;
      case 4:
      Serial.print("D");break;
      default:
      Serial.print("nothing matches."); break;
    }

    NFCReady = 2;// keep reading the next
    }
  else
    {
    Serial.print("Read Memory Block Command FAIL");
    NFCReady = 0;//reset the protocol 
    }

    return NFCReady;
}
 
void loop() {
  Serial.println("debug starts");
  if(NFCReady1 == 0)
    {
    delay(100);  
    NFCReady1 = SetProtocol_Command(SSPin1); // ISO 15693 settings
    }
 
  else if (NFCReady1 ==2)
    {
    delay(100);  
    NFCReady1 = Read_Memory(SSPin1);
    }
   Serial.print('\n');
   
   if(NFCReady2 == 0)
    {
    delay(100);  
    NFCReady2 = SetProtocol_Command(SSPin2); // ISO 15693 settings
    }
  
  else if (NFCReady2 ==2)
    {
    delay(100);  
    NFCReady2 = Read_Memory(SSPin2);
    }
    Serial.print('\n');
    
    if(NFCReady3 == 0)
    {
    delay(100);  
    NFCReady3 = SetProtocol_Command(SSPin3); // ISO 15693 settings
    }
  
  else if (NFCReady3 ==2)
    {
    delay(100);  
    NFCReady3 = Read_Memory(SSPin3);
    }
    Serial.print('\n');
    
    if(NFCReady4 == 0)
    {
    delay(100);  
    NFCReady4 = SetProtocol_Command(SSPin4); // ISO 15693 settings
    }

  else if (NFCReady4 ==2)
    {
    delay(100);  
    NFCReady4 = Read_Memory(SSPin4);
    }
  Serial.print('\n');
    delay(500);
}

