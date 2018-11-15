// rf69_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF69 class. RH_RF69 class does not provide for addressing or
// reliability, so you should only use RH_RF69  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf69_server.

#include <SPI.h>
#include <RH_RF69.h>

#define MOSI 11
#define MISO 12
#define SCK 13
#define SS 10

#define RFM69_CS      9 //10 //9
#define RFM69_IRQ     3 //2 //3
//#define RFM69_IRQN    1  // Pin 2 is IRQ 0!
#define RFM69_RST     14 //7
#define RFM69_RESET 14 //7

//#define LED           13 // Moteinos have LEDs on D9
#define FLASH_SS      9 //10 //9 // and FLASH SS on D8
#define SPI_CS               9 //10 //9 // SS is the SPI slave select pin, for WireLess Gate Shield - D9
#define RF69_IRQ_PIN          3 //2 //3 // INT1 on AVRs should be connected to DIO0 (ex on Atmega328 it's D3)
#define RFM69_RESET         14 //7

#define SPI_CLOCK_DIV128

RH_RF69 rf69(9, 3); // Adafruit Feather 32u4

void setup() 
{
  Serial.begin(9600);
  resetRFM69();
  delay(10);
  if (!rf69.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(434.0))
    Serial.println("setFrequency failed");

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  //rf69.setTxPower(14, true);

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
}

void loop()
{
  Serial.println("Sending to rf69_server");
  // Send a message to rf69_server
  uint8_t data[] = "Hello World!";
  rf69.send(data, sizeof(data));
  
  rf69.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf69.waitAvailableTimeout(500))
  { 
    // Should be a reply message for us now   
    if (rf69.recv(buf, &len))
    {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is rf69_server running?");
  }
  //delay(400);
}

void resetRFM69(){
  digitalWrite(RFM69_RESET, HIGH);
  delay(1);
  digitalWrite(RFM69_RESET, LOW);
  delay(10);
}

