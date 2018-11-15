// rf69_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing server
// with the RH_RF69 class. RH_RF69 class does not provide for addressing or
// reliability, so you should only use RH_RF69  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf69_client

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

#define FLASH_SS      9 //10 //9 // and FLASH SS on D8
#define SPI_CS               9 //10 //9 // SS is the SPI slave select pin, for WireLess Gate Shield - D9
#define RF69_IRQ_PIN          3 //2 //3 // INT1 on AVRs should be connected to DIO0 (ex on Atmega328 it's D3)
#define RFM69_RESET         14 //7

#define RFM69_CS      9
#define RFM69_IRQ     3
#define RFM69_IRQN    1
#define RFM69_RST     14
#define RFM69_RESET   14

RH_RF69 rf69(9, 3);

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
  rf69.setTxPower(14, true);

  // The encryption key has to be the same as the one in the client
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  
#if 0
  // For compat with RFM69 Struct_send
  rf69.setModemConfig(RH_RF69::GFSK_Rb250Fd250);
  rf69.setPreambleLength(3);
  uint8_t syncwords[] = { 0x2d, 0x64 };
  rf69.setSyncWords(syncwords, sizeof(syncwords));
  rf69.setEncryptionKey((uint8_t*)"thisIsEncryptKey");
#endif
}

void loop()
{
  if (rf69.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len))
    {
      RH_RF69::printBuffer("request: ", buf, len);
      Serial.print("got request: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf69.lastRssi(), DEC);
      
      // Send a reply
      uint8_t data[] = "And hello back to you";
      rf69.send(data, sizeof(data));
      rf69.waitPacketSent();
      Serial.println("Sent a reply");
    }
    else
    {
      Serial.println("recv failed");
    }
  }
}

void resetRFM69(){
  digitalWrite(RFM69_RESET, HIGH);
  delay(1);
  digitalWrite(RFM69_RESET, LOW);
  delay(10);
}

