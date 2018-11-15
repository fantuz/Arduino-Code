/* adaption to MEGA:
 *  - IRQ not 2 or 3 as occupied. board does use pin 8 and that is OK with my design)
 *  - SPI somewhere else (not 10-13 and not 50-53 even if 53 free)
 *  - ... WIP
 */

//Serial1 on pins 19 (RX) and 18 (TX), Serial2 on pins 17 (RX) and 16 (TX), Serial3 on pins 15 (RX) and 14 (TX)

/*
NFC Communication with the Solutions Cubed, LLC BM019 and an Arduino Uno.  The BM019 is a module that carries ST Micro's CR95HF, a serial to NFC converter.
 Arduino          BM019
 IRQ: Pin 9       DIN: pin 2
 SS: pin 10       SS: pin 3
 MOSI: pin 11     MOSI: pin 5 
 MISO: pin 12     MISO: pin4
 SCK: pin 13      SCK: pin 6
*/

#include <SPI.h>

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

#define SPI_CS_NFC  10
#define SS          10 //53    // PB0, pin 19
#define SPI_SS      10 //53    // PB0, pin 19

const int SSPin = 10;  // Slave Select pin
const int IRQPin = 8;  // Sends wake-up pulse

byte TXBuffer[40];     // transmit buffer
byte RXBuffer[40];     // receive buffer
byte NFCReady = 0;     // used to track NFC state

void setup() {
    
    pinMode(IRQPin, OUTPUT);
    digitalWrite(IRQPin, HIGH); // Wake up pulse
    pinMode(SSPin, OUTPUT);
    digitalWrite(SSPin, HIGH);

    Serial.begin(9600);
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    //SPI.setClockDivider(SPI_CLOCK_DIV16,32);
    SPI.setClockDivider(SPI_CLOCK_DIV32);
 
 // The CR95HF requires a wakeup pulse on its IRQ_IN pin before it will select UART or SPI mode.
 // The IRQ_IN pin is also the UART RX pin for DIN on the BM019 board.
 
    delay(10);                      // send a wake up
    digitalWrite(IRQPin, LOW);      // pulse to put the 
    delayMicroseconds(10);          // BM019 into SPI
    digitalWrite(IRQPin, HIGH);     // mode 
    delay(10);
}

/*

// CR95HF Commands
#define IDN               0x01
#define SELECT            0x02
#define POLL              0x03
#define SENDRECEIVE       0x04
#define LISTEN            0x05
#define SEND              0x06
#define ECHO              0x55

*/

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

void IDN_Command()
 {
 byte i = 0;

// step 1 send the command
  digitalWrite(SSPin, LOW);
  //delayMicroseconds(10);

  SPI.transfer(0);  // SPI control byte to send command to CR95HF
  SPI.transfer(1);  // IDN command
  SPI.transfer(0);  // length of data that follows is 0
  
  //delayMicroseconds(10);
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
  delayMicroseconds(10);
  SPI.transfer(0x02);   // SPI control byte for read         
  RXBuffer[0] = SPI.transfer(0);  // response code
  RXBuffer[1] = SPI.transfer(0);  // length of data
  for (i=0;i<RXBuffer[1];i++)      
      RXBuffer[i+2]=SPI.transfer(0);  // data
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
  
  //delayMicroseconds(10);
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
  //delayMicroseconds(10);
  SPI.transfer(0x02);   // SPI control byte for read         
  RXBuffer[0] = SPI.transfer(0);  // response code
  RXBuffer[1] = SPI.transfer(0);  // length of data
  delayMicroseconds(10);
  digitalWrite(SSPin, HIGH);
  //delay(10);
  
  if ((RXBuffer[0] == 0) & (RXBuffer[1] == 0))
  {
     Serial.println("PROTOCOL SET-");  //
     NFCReady = 1; // NFC is ready
  }
  else
  {
     //Serial.println("BAD RESPONSE TO SET PROTOCOL");
     NFCReady = 0; // NFC not ready
  }
  
  //NFCReady = 1;
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

void Inv_Tag_Info() {
 byte i = 0;

// step 1 send the command
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
  SPI.transfer(0x04);  // Send Receive CR95HF command
  //SPI.transfer(0x00);  // length of data that follows is 0
  //SPI.transfer(0x26);  // request Flags byte
  SPI.transfer(0x2B);  // TAG INFO for ISO/IEC 15693
  SPI.transfer(0x00);  // mask length for inventory command
  delayMicroseconds(10);
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

  if (RXBuffer[0] == 128) {  
    Serial.println("TAG DETECTED");
    Serial.print("UID: ");
    for(i=11;i>=4;i--)
    {
      Serial.print(RXBuffer[i],HEX);
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

void Inventory_Command() {
 byte i = 0;

// step 1 send the command
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
  SPI.transfer(0x04);  // Send Receive CR95HF command
  SPI.transfer(0x03);  // length of data that follows is 0
  SPI.transfer(0x26);  // request Flags byte
  SPI.transfer(0x01);  // Inventory Command for ISO/IEC 15693
  SPI.transfer(0x00);  // mask length for inventory command
  //delayMicroseconds(10);
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

  if (RXBuffer[0] == 128) {  
    Serial.println("TAG DETECTED");
    Serial.print("UID: ");
    for(i=11;i>=4;i--)
    {
      Serial.print(RXBuffer[i],HEX);
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
  SPI.transfer(0x01);  // Send Receive CR95HF command //// 0x04
  SPI.transfer(0x02);  // length of data that follows is 0
  SPI.transfer(0x26);  // request Flags byte
  SPI.transfer(0xBC);  // MCU VERSION COMMAND
  SPI.transfer(0x00);  // mask length for inventory command
  delayMicroseconds(10);
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
  delayMicroseconds(10);
  digitalWrite(SSPin, HIGH);
  delay(10);

//  if ((RXBuffer[0] == 0) & (RXBuffer[1] == 15))

  Serial.print("RESPONSE CODE: ");
  Serial.print(RXBuffer[0]);
  Serial.print(" LENGTH: ");
  Serial.println(RXBuffer[1]);
  Serial.print("MCU VER: ");
  for(i=2;i<(RXBuffer[1]);i++)
  {
    Serial.print(RXBuffer[i],HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("MCU ARR: ");
  Serial.print(RXBuffer[RXBuffer[1]],HEX);
  Serial.print(RXBuffer[RXBuffer[1]+1],HEX);
  Serial.println(" ");

    //    Hex2Asc(StringReply,&RespArray[1],RespArray[2]+2);

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

void Mcu_Echo() {
 byte i = 0;

// step 1 send the command
  digitalWrite(SSPin, LOW);
  SPI.transfer(0x00);  // SPI control byte to send command to CR95HF
  SPI.transfer(0x55);  // Send Receive CR95HF command //// 0x04
  //SPI.transfer(0x02);  // length of data that follows is 0
  //SPI.transfer(0x26);  // request Flags byte
  //SPI.transfer(0xBC);  // MCU VERSION COMMAND
  //SPI.transfer(0x00);  // mask length for inventory command
  delayMicroseconds(10);
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
  delayMicroseconds(10);
  digitalWrite(SSPin, HIGH);
  delay(10);

//  if ((RXBuffer[0] == 0) & (RXBuffer[1] == 15))

  Serial.print("PING: ");
  Serial.println(RXBuffer[0]);
  Serial.print("PONG: ");
  for(i=2;i<(RXBuffer[1]);i++)
  {
    Serial.print(RXBuffer[i],HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("BAFF: ");
  Serial.print(RXBuffer[RXBuffer[1]],HEX);
  Serial.print(RXBuffer[RXBuffer[1]+1],HEX);
  Serial.println(" ");

    //    Hex2Asc(StringReply,&RespArray[1],RespArray[2]+2);

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
/*

int CR95HFlib_MCUVer(char* StringReply)
{
  unsigned char CmdArray[MAX_BUFFER_SIZE];
        unsigned char RespArray[MAX_BUFFER_SIZE];
    
  if(!mgrObj.IsReadyforCommunication)
  {
    return STM32_connection_error;
  }
  else
  {
    CmdArray[0]= ID_SEND_request_to_MCU;  
    CmdArray[1]= 0xBC;
    // Transmits the command 
    mgrObj.WriteToDevice(CmdArray,2);//size of data to write should not be hardcoded ??
    // Recovers the reply 
    mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);   
    
    Hex2Asc(StringReply,&RespArray[1],RespArray[2]+2);
    return 0;
          
  }
}

*/

void loop() {
  
  if(NFCReady == 0)
  {
    IDN_Command();  // reads the CR95HF ID
    delay(500);
    SetProtocol_Command(); // ISO 15693 settings
    delay(500);
  }
  else
  {
    Inventory_Command();
    delay(500);
    //Mcu_Ver();
    //Inv_Tag_Info;
    //Mcu_Echo();
    //delay(500);
  }  
  
}


/*

* File Name        : libcr95hf.cpp
* Author           : IMS Systems Lab & Technical Marketing
* Version          : 1.0
* Date             : 17th May, 2014
* Description      : CR95HF library on Linux


#include <HIDManager.h>
#include <string.h>
#include <stdio.h>
*/
/*
// HID Commands
# define ID_SPI_SEND_DATA    0x01
# define ID_SEND_request_to_MCU 0x02
# define ID_SEND_HID_RESPONSE 0x07 

// CR95HF Commands
#define IDN               0x01
#define SELECT            0x02
#define POLL              0x03
#define SENDRECEIVE       0x04
#define LISTEN            0x05
#define SEND              0x06
#define ECHO              0x55

// CR95 Demo Reader Errors returned on commands
#define STM32_error             0x01
#define empty_argument_error    0x02
#define cmd_parameter_error     0x03
#define communication_error     0x04
#define STM32_connection_error  0x05

// Max buffer size = 256 data bytes + 2 cmd bytes
#define MAX_BUFFER_SIZE 64 

int Hex2Asc(char*,unsigned char*,int);
int Asc2Hex(unsigned char *,char *,int);

Ccr95HIDManager mgrObj;

int CR95HFlib_Idn(unsigned char* Reply)
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
    mgrObj.WriteToDevice(CmdArray,3);//size of data to write should not be hardcoded ??
    // Recovers the reply
    mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);
    
    return 0 ; 
  }
}

int CR95HFlib_USBConnect()
{
  
  bool isSuccess=mgrObj.MakeDeviceReady();
  if(isSuccess)
  {
    
    return 0;
  }
  else
  {
    return STM32_error;
  }
}

int CR95HFlib_MCUVer(char* StringReply)
{
  unsigned char CmdArray[MAX_BUFFER_SIZE];
        unsigned char RespArray[MAX_BUFFER_SIZE];
    
  if(!mgrObj.IsReadyforCommunication)
  {
    return STM32_connection_error;
  }
  else
  {
    CmdArray[0]= ID_SEND_request_to_MCU;  
    CmdArray[1]= 0xBC;
    // Transmits the command 
    mgrObj.WriteToDevice(CmdArray,2);//size of data to write should not be hardcoded ??
    // Recovers the reply 
    mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);   
    
    Hex2Asc(StringReply,&RespArray[1],RespArray[2]+2);
    return 0;
          
  }
}

int  CR95HFlib_Echo(char* StringReply)
{
  
  unsigned char CmdArray[MAX_BUFFER_SIZE], RespArray[MAX_BUFFER_SIZE];
   
  if(!mgrObj.IsReadyforCommunication)
  {
    return STM32_connection_error ;
      }

    else
  {
    CmdArray[0] = ID_SPI_SEND_DATA;
    CmdArray[1] = ECHO;
    // Transmits the command
    mgrObj.WriteToDevice(CmdArray,2);
    //size of data to write should not be  ??
    
    // Recovers the reply
    mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);
    Hex2Asc(StringReply,&RespArray[1],RespArray[2]+2);    
    return 0;
  }
}

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

int  CR95HFlib_Select(char* StringCmd, char* StringReply)  //select protocol
{
  unsigned char CmdArray[MAX_BUFFER_SIZE], RespArray[MAX_BUFFER_SIZE], RespCode;
        
  if(!mgrObj.IsReadyforCommunication)
  {
    return STM32_connection_error;
  }

  else 
  {
    // Checks if the string is empty 
    if(strlen(StringCmd) == 0)
      {
      // An error occured 
      return empty_argument_error ;
      }
    else
    {
      CmdArray[0] = ID_SPI_SEND_DATA;
      CmdArray[1] = SELECT;
      CmdArray[2] = strlen(StringCmd)/2;//??
      // append passed string to the command array
      Asc2Hex(&CmdArray[3],StringCmd,strlen(StringCmd));  
    
      // Transmits the command
      mgrObj.WriteToDevice(CmdArray,strlen(StringCmd)+3);//?? 
      // Recovers the reply
      mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);
      RespCode = RespArray[1];
      if(RespCode != 0x00)
      { 
        if (RespCode == 0x82)
        {
          strcpy(StringReply , "8200 : Invalid command length");
        }
        else if ( RespCode == 0x83 ) 
        {
          strcpy(StringReply , "8300 : Invalid protocol");
        } 

        return cmd_parameter_error ; 
      }
      else
        {
          Hex2Asc(StringReply, &RespArray[1], RespArray[2] + 2);
          return 0 ; 
        }
    }//end else
  
  }//End else

}

int  CR95HFlib_SendReceive(char *StringCmd, char *StringReply) 
{

  unsigned char CmdArray[MAX_BUFFER_SIZE], RespArray[MAX_BUFFER_SIZE], RespCode;
      
  if(!mgrObj.IsReadyforCommunication)
  {
    return STM32_connection_error;
  }
  else
  {
    
    if(strlen(StringCmd) == 0)
      {
      CmdArray[0] = ID_SPI_SEND_DATA;
      CmdArray[1] = SENDRECEIVE;
      CmdArray[2] = 0x0;
      }
    else  { 
      CmdArray[0] = ID_SPI_SEND_DATA;
      CmdArray[1] = SENDRECEIVE;
      CmdArray[2] = strlen(StringCmd)/2;
      
      Asc2Hex(&CmdArray[3] , StringCmd, strlen(StringCmd));
      }
      
    mgrObj.WriteToDevice(CmdArray,CmdArray[2]+3);//?? 
      
      
    mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);
      
    RespCode = RespArray[1];
    if((RespCode != 0x80) && (RespCode != 0x90))
      { 
        if ( RespCode == 0x86 ) 
          {
          strcpy(StringReply , "8600 : Communication error");
          } 
        else if ( RespCode == 0x87 ) 
          {
          strcpy(StringReply , "8700 : Frame wait time out OR no tag");
          } 
        else if ( RespCode == 0x88 ) 
          {
          strcpy(StringReply , "8800 : Invalid Start Of Frame");        
          } 
        else if ( RespCode == 0x89 ) 
          {
          strcpy(StringReply , "8900 : Receive buffer overflow (too many bytes received)");
          } 

        else if ( RespCode == 0x8A ) 
          {
          strcpy(StringReply , "8A00: Framing error (start bit=0, stop bit=1)");
          } 
        else if ( RespCode == 0x8B ) 
          {
          strcpy(StringReply , "8B00 : EGT time out (for Iso14443-B)");       
          } 
        else if ( RespCode == 0x8C ) 
          {
          strcpy(StringReply , "8C00 : Invalid length. Used in Felica, when field Length < 3");
          } 
        else if ( RespCode == 0x8D ) 
          {
          strcpy(StringReply , "8D00 : CRC error (Used in Felica protocol)");
          } 
        else if ( RespCode == 0x8E ) 
          {
          strcpy(StringReply , "8E00 : Reception lost without EOF received"); 
          } 
        else 
          {
          strcpy(StringReply , "Unknow error");
          //StringReply = "Unknow error";  //?? why it didnt work
          }
        return communication_error ; 
      }
    else
      {
        
        Hex2Asc(StringReply, &RespArray[1], RespArray[2] + 2);
        return 0; 
      }
  }   
}

int CR95HFlib_Read_Block(int RegAdd, unsigned char* DataReceived)

{
char arrReceived[MAX_BUFFER_SIZE];
char cmdRead[40];

unsigned char nibbleArr[4];

//char cmdArray[40];

unsigned char maskReg1 = 0XF0;

unsigned char maskReg2 = 0X0F;

unsigned char arrbyte[2];

arrbyte[0]=(
unsigned char)RegAdd;

arrbyte[1]=(
unsigned char)(RegAdd>>8);

nibbleArr[0] = (
unsigned char)((arrbyte[0] & maskReg1) >> 4);

nibbleArr[1] = (
unsigned char)(arrbyte[0] & maskReg2);

nibbleArr[2] = (
unsigned char)((arrbyte[1] & maskReg2) >> 4);

nibbleArr[3] = (
unsigned char)(arrbyte[1] & maskReg2);

cmdRead[0] =
'0';

cmdRead[1] =
'A';

cmdRead[2] =
'2';

cmdRead[3] =
'0';

int i,j;
for (j = 4, i = 0; j <= 7; j++, i++)
{
  char hexVal = ' ';
  if (nibbleArr[i] < 10)
  {
    hexVal=nibbleArr[i]+48;
  }
  else 
  {
    hexVal=nibbleArr[i]+55;
  }
cmdRead[j] = hexVal;    
}
cmdRead[j]=0;

// send command to get data

int istatus = CR95HFlib_SendReceive(cmdRead, arrReceived);
Asc2Hex(DataReceived , arrReceived, strlen(arrReceived));

return istatus;

}

int CR95HFlib_Write_Block(int RegAdd, unsigned char* strTagAnswer,unsigned char* strBytestowrite)
{
char arrReceived[MAX_BUFFER_SIZE];
char cmdWrite[40];

unsigned char nibbleArr[4];

//char cmdArray[40];

unsigned char maskReg1 = 0XF0;

unsigned char maskReg2 = 0X0F;

unsigned char arrbyte[2];

arrbyte[0]=(
unsigned char)RegAdd;

arrbyte[1]=(
unsigned char)(RegAdd>>8);

nibbleArr[0] = (
unsigned char)((arrbyte[0] & maskReg1) >> 4);

nibbleArr[1] = (
unsigned char)(arrbyte[0] & maskReg2);

nibbleArr[2] = (
unsigned char)((arrbyte[1] & maskReg2) >> 4);

nibbleArr[3] = (
unsigned char)(arrbyte[1] & maskReg2);

cmdWrite[0] =
'0';

cmdWrite[1] =
'A';

cmdWrite[2] =
'2';

cmdWrite[3] =
'1';

int i,j;
for (j = 4, i = 0; j <= 7; j++, i++)
{
  char hexVal = ' ';
  if (nibbleArr[i] < 10)
  {
    hexVal=nibbleArr[i]+48;
  }
  else 
  {
    hexVal=nibbleArr[i]+55;
  }
cmdWrite[j] = hexVal;   
}

cmdWrite[j]=0;
Hex2Asc(&cmdWrite[j], strBytestowrite,4);
int istatus = CR95HFlib_SendReceive(cmdWrite, arrReceived);
return istatus;
}

int CR95HFlib_ResetSPI(char* StringReply)
{
  unsigned char CmdArray[MAX_BUFFER_SIZE];
        unsigned char RespArray[MAX_BUFFER_SIZE];
    
  if(!mgrObj.IsReadyforCommunication)
  {
    return STM32_connection_error;
  }
  else
  {
    CmdArray[0]= ID_SEND_request_to_MCU;  
    CmdArray[1]= 0xBD;
    // Transmits the command 
    mgrObj.WriteToDevice(CmdArray,2);//size of data to write should not be hardcoded ??
    // Recovers the reply 
    mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);   
    
    Hex2Asc(StringReply,&RespArray[1],RespArray[2]+2);
    return 0;
          
  }
}

int CR95HFlib_SendIRQPulse(char* StringReply)
{
  unsigned char CmdArray[MAX_BUFFER_SIZE];
        unsigned char RespArray[MAX_BUFFER_SIZE];
    
  if(!mgrObj.IsReadyforCommunication)
  {
    return STM32_connection_error;
  }
  else
  {
    CmdArray[0]= ID_SEND_request_to_MCU;  
    CmdArray[1]= 0xBE;
    // Transmits the command 
    mgrObj.WriteToDevice(CmdArray,2);//size of data to write should not be hardcoded ??
    // Recovers the reply 
    mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);   
    
    Hex2Asc(StringReply,&RespArray[1],RespArray[2]+2);
    return 0;         
  }
}

int CR95HFlib_getInterfacePinState(char* StringReply)
{
  unsigned char CmdArray[MAX_BUFFER_SIZE];
        unsigned char RespArray[MAX_BUFFER_SIZE];
    
  if(!mgrObj.IsReadyforCommunication)
  {
    return STM32_connection_error;
  }
  else
  {
    CmdArray[0]= ID_SEND_request_to_MCU;  
    CmdArray[1]= 0xBA;
    // Transmits the command 
    mgrObj.WriteToDevice(CmdArray,2);//size of data to write should not be hardcoded ??
    // Recovers the reply 
    mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);   
    
    Hex2Asc(StringReply,&RespArray[1],RespArray[2]+2);
    return 0;         
  }
}

int CR95HFlib_SendNSSPulse(char* StringReply)
{
  unsigned char CmdArray[MAX_BUFFER_SIZE];
        unsigned char RespArray[MAX_BUFFER_SIZE];
    
  if(!mgrObj.IsReadyforCommunication)
  {
    return STM32_connection_error;
  }
  else
  {
    CmdArray[0]= ID_SEND_request_to_MCU;  
    CmdArray[1]= 0xB9;
    // Transmits the command 
    mgrObj.WriteToDevice(CmdArray,2);//size of data to write should not be hardcoded ??
    // Recovers the reply 
    mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);   
    
    Hex2Asc(StringReply,&RespArray[1],RespArray[2]+2);
    return 0;         
  }
}

int CR95HFlib_STCmd(char* stringCmd, char* stringReply)
{
  unsigned char CmdArray[MAX_BUFFER_SIZE];
        unsigned char RespArray[MAX_BUFFER_SIZE];
    
  if(!mgrObj.IsReadyforCommunication)
  {
    return STM32_connection_error;
  }
  else
  {
    if(strlen(stringCmd)== 0)
    {
      return empty_argument_error;
    }
    Asc2Hex(&CmdArray[0],stringCmd,strlen(stringCmd));
    
    // Transmits the command 
    mgrObj.WriteToDevice(CmdArray,(strlen(stringCmd))/2);//size of data to write should not be hardcoded ??
    // Recovers the reply 
    mgrObj.ReadFromDevice(RespArray,MAX_BUFFER_SIZE);   
    
    Hex2Asc(stringReply,&RespArray[1],RespArray[2]+2);
    return 0;
          
  }
}

//
// Hexa to Ascii 0x22 -> "22"
int Hex2Asc(char *Dest,unsigned char *Src,int SrcLen)
{
  int i;
  for ( i = 0; i < SrcLen; i ++ )
  {
    sprintf(Dest + i * 2,"%02X",Src[i]);
  }
  Dest[i * 2] = 0;
  return 0;
}

// Ascii to Hexa "22" -> 0x22
int Asc2Hex(unsigned char *Dest,char *Src,int SrcLen)
{
  int i;
  for ( i = 0; i < SrcLen / 2; i ++ )
  {
    sscanf(Src + i * 2,"%02X",(unsigned int*)&Dest[i]);
  }
  return 0;
}

*/
