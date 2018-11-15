// http://blog.solutions-cubed.com/near-field-communication-nfc-with-the-arduino/
// http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/DM00025644.pdf

#include <SPI.h>

// Write, poll, read, reset
#define   CMD             0x00
#define   POLL            0x03
#define   READ            0x02
#define   RESET           0x01

// Commands
#define   IDN             0x01  // gives brief information about CR95HF and its revision
#define   PROTOCOLSELECT  0x02  // Selects RF communication protocol and prepares CR95HF for communication
#define   SENDRECEIVE     0x04  // Sends data to tag and receives its reply
#define   Idle            0x07  // Switches the CR95HF into low consumption mode
#define   RDREG           0x08  // Read the wakeup register
#define   WRREG           0x09  // Set analog register config address, set timer window, set autodetect filter, configure HF2RF bit 
#define   BAUDRATE        0x0A  // changes the UART baud rate
#define   ECHO            0x55  // Verifies the possibility of communication between a Host and the CR95HF

#define SPI_CS_NFC 10;
const int _SSpin = 10; //53; // CS
const int _IRQpin = 2; //9;

// INT_in : 2, INT_out : 6
/*
UART_RX/IRQ_OUT  2
UART_TX/IRQ_IN   8
Interface_Pin    9
SPI_CS_NFC      10
*/

byte _rxBuffer[16]; // buffers for data only
byte _txBuffer[16];

void SendCmd(byte command, byte dataLen)
{
  digitalWrite(_SSpin, LOW);
  SPI.transfer(CMD);
  SPI.transfer(command); 
  SerialPrint("SendCommand", command);                         // <---------serial
  if (command != ECHO)
  {
    SPI.transfer(dataLen);
    SerialPrint("SendDataLen", dataLen);                       // <---------serial

    for (byte i = 0; i < dataLen; i++)
    {
      SPI.transfer(_txBuffer[i]);
      SerialPrint("_txBuffer", _txBuffer[i]);                  // <---------serial
    }
  }
  digitalWrite(_SSpin, HIGH);
  delay(10);
}

void PollSlave()
{
  byte tmp = 0;
  byte tmp2 = 0;
  
  digitalWrite(_SSpin, LOW);
  while (tmp != 1)
  {
    tmp = SPI.transfer(POLL);
    SerialPrint("Poll tmp", tmp);                            // <---------serial
    tmp2 = (tmp & 0x08) >> 3;
    SerialPrint("Poll tmp2", tmp2);                          // <---------serial
  }
  digitalWrite(_SSpin, HIGH);
  delay(10);
}

void ReadMsg()
{
  byte res = 0;
  byte len = 0;

  digitalWrite(_SSpin, LOW);
  SPI.transfer(READ);
  res = SPI.transfer(0);
  SerialPrint("res", res);                    // <---------serial
  delay(1000);
  len = SPI.transfer(0);
  SerialPrint("len", len);                    // <---------serial
  delay(1000);

  for (byte i = 0; i < len; i++)
  {
    Serial.println(SPI.transfer(0));
    _rxBuffer[i] = SPI.transfer(0);
    SerialPrint("_rxBuffer", _rxBuffer[i]);                    // <---------serial
  }
  digitalWrite(_SSpin, HIGH);
}

void WakeupCR95HF()
{
  digitalWrite(_IRQpin, LOW);
  delay(10);
  digitalWrite(_IRQpin, HIGH);
  delay(10);
}

void SerialPrint(String nm, byte item)
{
  Serial.println(nm + ": " + item);
  Serial.println();
}

void SendRecv()
{
  _txBuffer[0] = 0x03;
  _txBuffer[1] = 0x02;
  _txBuffer[2] = 0x20;
  _txBuffer[3] = 0x00;
  //SendCmd(SENDRECV, 4);
  SendCmd(SENDRECEIVE, 4);
  PollSlave();
  ReadMsg();
}

bool EchoResponse()
{
  SendCmd(ECHO, 0);
  PollSlave();
  ReadMsg();

  byte res = 0;
  byte len = 0;

  digitalWrite(_SSpin, LOW);
  SPI.transfer(0x02);
  res = SPI.transfer(8);
  delay(100);
  SerialPrint("res", res);                    // <---------serial
  digitalWrite(_SSpin, HIGH);
  delay(100);
  if (res == ECHO)
  { 
    return true;
  }
  return false;
}

void ReadCR95HF_ID()
{
  SendCmd(IDN, 0);
  PollSlave();
  ReadMsg();
}

void FieldOff()
{
  _txBuffer[0] = 0x00;
  _txBuffer[1] = 0x00;
  SendCmd(PROTOCOLSELECT, 0x02);
  PollSlave();
  ReadMsg();
}

void SetProtocol() // Field on
{
  _txBuffer[0] = 0x01;
  _txBuffer[1] = 0x01;
  SendCmd(PROTOCOLSELECT, 0x02);
  PollSlave();
  ReadMsg();

  if (_rxBuffer[0] != 0x82 || _rxBuffer[0] != 0x83)
  {
    Serial.println("Successfull");
  }
}

void setup()
{
  // SSI_0 and 1 needed?

  Serial.begin(9600);                                // <---------serial

  pinMode(_SSpin, OUTPUT);
  pinMode(_IRQpin, OUTPUT);

  digitalWrite(_IRQpin, HIGH); // wakeup
  delay(10);
  digitalWrite(_SSpin, HIGH);  // slave select, high for not listening
  delay(10);
 
  WakeupCR95HF();
  
  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

/*
SPI.begin(); 
SPI.setBitOrder(MSBFIRST); 
SPI.setDataMode(SPI_MODE0); 
SPI.setClockDivider(SPI_BAUD_PCLK_DIV_16); // 72 / 16 = 4.5 MHz speed 
*/

  // send reset command just in case the CR95HF hasn't been powered off
  digitalWrite(_SSpin, LOW);
  
  digitalWrite(_SSpin, HIGH);

  EchoResponse();
  while(!EchoResponse())
  {
    WakeupCR95HF();
    Serial.println("Reset");
  }

  ReadCR95HF_ID();
  //Calibration();
  SetProtocol();
}

void loop()
{
  delay(50);
}

void Calibration()
{
  _txBuffer[0] = 0x03;
  _txBuffer[1] = 0xA1;
  _txBuffer[2] = 0x00;
  _txBuffer[3] = 0xF8;
  _txBuffer[4] = 0x01;
  _txBuffer[5] = 0x18;
  _txBuffer[6] = 0x00;
  _txBuffer[7] = 0x20;
  _txBuffer[8] = 0x60;
  _txBuffer[9] = 0x60;
  _txBuffer[10] = 0x00;
  _txBuffer[11] = 0x00;
  _txBuffer[12] = 0x3F;
  _txBuffer[13] = 0x01;
  SendCmd(Idle, 0x0E);
  PollSlave();
  ReadMsg();

  _txBuffer[11] = 0xFC;
  SendCmd(Idle, 0x0E);
  PollSlave();
  ReadMsg();

  _txBuffer[11] = 0x7C;
  SendCmd(Idle, 0x0E);
  PollSlave();
  ReadMsg();

  _txBuffer[11] = 0x3C;
  SendCmd(Idle, 0x0E);
  PollSlave();
  ReadMsg();

  _txBuffer[11] = 0x5C;
  SendCmd(Idle, 0x0E);
  PollSlave();
  ReadMsg();

  _txBuffer[11] = 0x6C;
  SendCmd(Idle, 0x0E);
  PollSlave();
  ReadMsg();

  _txBuffer[11] = 0x74;
  SendCmd(Idle, 0x0E);
  PollSlave();
  ReadMsg();

  _txBuffer[11] = 0x70;
  SendCmd(Idle, 0x0E);
  PollSlave();
  ReadMsg();
}

