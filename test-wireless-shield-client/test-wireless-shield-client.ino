
#include <RFM69.h>
#include <RFM69_ATC.h>     //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <SPIFlash.h>      //get it here: https://www.github.com/lowpowerlab/spiflash
#include "RF24.h"
#include <IRremote.h>
#include <Wire.h>
#include "RTClib.h"

#define IS_RFM69HCW false
#define NODEID      1
#define NETWORKID   100
#define GATEWAYID   2
#define FREQUENCY   RF69_433MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define KEY         "thisIsEncryptKey" //has to be same 16 characters/bytes on all nodes, not more not less!
#define LED         6
#define SERIAL_BAUD 9600
#define ACK_TIME    100  // # of ms to wait for an ack
 
#define RF69_IRQ_PIN  3 //3 // INT1 on AVRs should be connected to DIO0 (ex on Atmega328 it's D3)

#define RFM69_CS      9 //9
#define SS            8 //9
#define FLASH_SS      8 //9 // and FLASH SS on D8
#define SPI_CS        9 //9 // SS is the SPI slave select pin, for WireLess Gate Shield - D9
#define RFM69_IRQ     3 //3
#define RFM69_IRQN    1  // Pin 2 is IRQ 0!
#define RFM69_RST     14
#define RFM69_RESET   14 //7
 
RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);
SPIFlash flash(FLASH_SS, 0x0000);
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network
 
typedef struct {    
  int           nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  float         temp;   //temperature maybe?
}

Payload;
Payload theData;
 
void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(RFM69_RESET, OUTPUT);
  pinMode(RFM69_IRQ, INPUT);
  delay(10);
  resetRFM69();
  radio.setCS(SPI_CS);
  delay(10);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  //radio.setHighPower(); //uncomment only for RFM69HW!
  radio.encrypt(KEY);
  radio.promiscuous(promiscuousMode);
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
}
 
byte ackCount=0;
void loop() {

  if (Serial.available() > 0)
  {
    char input = Serial.read();
    if (input == 'r') //d=dump all register values
      radio.readAllRegs();
    if (input == 'E') //E=enable encryption
      radio.encrypt(KEY);
    if (input == 'e') //e=disable encryption
      radio.encrypt(null);
    if (input == 'p')
    {
      promiscuousMode = !promiscuousMode;
      radio.promiscuous(promiscuousMode);
      Serial.print("Promiscuous mode ");Serial.println(promiscuousMode ? "on" : "off");
    }
 
    if (input == 'd') //d=dump flash area
    {
      Serial.println("Flash content:");
      int counter = 0;
 
      while(counter<=256){
        Serial.print(flash.readByte(counter++), HEX);
        //Serial.print('.');
        //counter++;
      }
      while(flash.busy());
      Serial.println();
    }
    if (input == 'D')
    {
      Serial.print("Deleting Flash chip content... ");
      flash.chipErase();
      while(flash.busy());
      Serial.println("DONE");
    }
    if (input == 'i')
    {
      Serial.print("DeviceID: ");
      word jedecid = flash.readDeviceId();
      Serial.println(jedecid, HEX);
    }
  }
  
  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    Serial.print(" [RX_RSSI:");Serial.print(radio.readRSSI());Serial.print("]");
    if (promiscuousMode)
  {
      Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
    }
 
    if (radio.DATALEN != sizeof(Payload))
      Serial.print("Invalid payload received, not matching Payload struct!");
    else
    {
      theData = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
      Serial.print(" nodeId=");
      Serial.print(theData.nodeId);
      Serial.print(" uptime=");
      Serial.print(theData.uptime);
      Serial.print(" temp=");
      Serial.print(theData.temp);
    }
 
    if (radio.ACK_REQUESTED)
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
      Serial.print(" - ACK sent.");
 
      // When a node requests an ACK, respond to the ACK
      // and also send a packet requesting an ACK (every 3rd one only)
      // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
      if (ackCount++%3==0)
      {
        Serial.print(" Pinging node ");
        Serial.print(theNodeID);
        Serial.print(" - ACK...");
        delay(3); //need this when sending right after reception .. ?
        if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 2))  // 0 = only 1 attempt, no retries
          Serial.print("ok!");
        else Serial.print("nothing");
      }
    }
    Serial.println();
    Blink(LED,3);
  }
}
 
void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
 
void resetRFM69(){
  digitalWrite(RFM69_RESET, HIGH);
  delay(1);
  digitalWrite(RFM69_RESET, LOW);
  delay(10);
}

