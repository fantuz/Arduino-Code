// rf69_reliable_datagram_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging client
// with the RHReliableDatagram class, using the RH_RF69 driver to control a RF69 radio.
// It is designed to work with the other example rf69_reliable_datagram_server
// Tested on Moteino with RFM69 http://lowpowerlab.com/moteino/
// Tested on miniWireless with RFM69 www.anarduino.com/miniwireless
// Tested on Teensy 3.1 with RF69 on PJRC breakout board

#include <RHReliableDatagram.h>
#include <RH_RF69.h>
#include <SPI.h>

#define SPI_CS  9
 
#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2
#define FREQUENCY RF69_433MHZ
//#define FREQUENCY RF69_915MHZ
#define RF69_FREQ 433.92

#define MOSI 11
#define MISO 12
#define SCK 13
 
#define RFM69_INT     3
#define RFM69_RST     14 //A0 //14 //7
#define LED           6

#define RFM69_NSS 9
#define RF69_IRQ_PIN          3
#define RF69_IRQ_NUM          1
#define RFM69_DIO0 3
#define RFM69_RESET 14 //A0
// Singleton instance of the radio driver
RH_RF69 driver(9,3);
//RH_RF69 driver(15, 16); // For RF69 on PJRC breakout board with Teensy 3.1
//RH_RF69 driver(4, 2); // For MoteinoMEGA https://lowpowerlab.com/shop/moteinomega
//RH_RF69 driver(8, 7); // Adafruit Feather 32u4

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

void setup() 
{
  Serial.begin(9600);
  //while (!Serial) 
  //  ;
  
  pinMode(LED, OUTPUT);
 
  pinMode(RFM69_NSS, OUTPUT);
  //pinMode(7, OUTPUT);
  //pinMode(8, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCK, OUTPUT);

   // Ensure serial flash is not interfering with radio communication on SPI bus
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  
  pinMode(RFM69_RESET, OUTPUT);
  pinMode(RFM69_DIO0, INPUT);

  delay(100);
  if (!manager.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)

  driver.setFrequency(433.92);
  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  driver.setTxPower(14, true);

}

uint8_t data[] = "Hello World!";
// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

void loop()
{
  Serial.println("Sending to rf69_reliable_datagram_server");
    
  // Send a message to manager_server
  if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    //else
    //{
    //  Serial.println("No reply, is rf69_reliable_datagram_server running?");
    //}
  }
  //else
  //  Serial.println("sendtoWait failed");
  delay(200);
}

