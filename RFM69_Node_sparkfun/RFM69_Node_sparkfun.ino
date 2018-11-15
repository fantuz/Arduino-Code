// Sample RFM69 sender/node sketch, with ACK and optional encryption, and Automatic Transmission Control
// Sends periodic messages of increasing length to gateway (id=1). It also looks for an onboard FLASH chip, if present
// Copyright Felix Rusu 2016, http://www.LowPowerLab.com/contact

#include <RFM69.h>         //get it here: https://www.github.com/lowpowerlab/rfm69
#include <RFM69_ATC.h>     //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPIFlash.h>      //get it here: https://www.github.com/lowpowerlab/spiflash
#include <SPI.h>           //included with Arduino IDE install (www.arduino.cc)
#include <LiquidCrystal.h>

#define RFM69_IRQ     2 //3
#define RFM69_INT     2
#define RF69_IRQ_PIN  2 //3 // INT1 on AVRs should be connected to DIO0 (ex on Atmega328 it's D3)
#define RFM69_IRQN    0 //1  // Pin 2 is IRQ 0!

#define RFM69_RST     53 //27 //7 //14
#define RFM69_RESET   53 //27 //7 //14

#define MOSI      51 //11
#define MISO      50 //12
#define SCK       52 //13

#define RFM69_CS  53 //10 //9
#define SS        53 //10 //9
#define SPI_CS    53 //10 //9 // SS is the SPI slave select pin, for WireLess Gate Shield - D9
#define FLASH_SS  53 //10 //9 // and FLASH SS on D8

#define LED          13 //13 //6 // Moteinos have LEDs on D9

#define SERIAL_BAUD   9600

#define NODEID        2    //must be unique for each node on same network (range up to 254, 255 is used for broadcast)
#define NETWORKID     100  //the same on all nodes that talk to each other (range up to 255)
#define GATEWAYID     1

#define FREQUENCY   RF69_433MHZ
#define RF69_FREQ 434.0
#define FQTX 434000000
//#define FREQUENCY   RF69_868MHZ
//#define RF69_FREQ 868.0
//#define FQTX 868000000
//#define FREQUENCY     RF69_915MHZ
//#define RF69_FREQ 915.0
//#define FQTX 915000000

#define ENCRYPTKEY    "thisIsEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define KEY         "thisIsEncryptKey" //has to be same 16 characters/bytes on all nodes, not more not less!
//#define IS_RFM69HW_HCW  //uncomment only for RFM69HW/HCW! Leave out if you have RFM69W/CW!

//Auto Transmission Control - dials down transmit power to save battery
//Usually you do not need to always transmit at max output power
//By reducing TX power even a little you save a significant amount of battery power
//This setting enables this gateway to work with remote nodes that have ATC enabled to
//dial their power down to only the required level (ATC_RSSI)
//#define ENABLE_ATC    //comment out this line to disable AUTO TRANSMISSION CONTROL
//#define ATC_RSSI      -80

/*
#ifdef __AVR_ATmega1284P__
  #define LED           15 // Moteino MEGAs have LEDs on D15
  #define FLASH_SS      23 // and FLASH SS on D23
#else
  #define LED           9 // Moteinos have LEDs on D9
  #define FLASH_SS      8 // and FLASH SS on D8
#endif
*/

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; //LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int TRANSMITPERIOD = 250; //transmit a packet to gateway so often (in ms)
char payload[] = "123 ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char buff[20];
byte sendSize=0;
boolean requestACK = true;

//SPIFlash flash(FLASH_SS, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)
SPIFlash flash(FLASH_SS, 0x0000); //EF30 for 4mbit  Windbond chip (W25X40CL)

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif

void setup() {
  Serial.begin(SERIAL_BAUD);
  //delay(10);
  resetRFM69();
  radio.setCS(RFM69_CS); // NSS - D9 //10
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW_HCW
  radio.setHighPower(); //must include this only for RFM69HW/HCW!
#endif
  radio.encrypt(ENCRYPTKEY);
  radio.setFrequency(FQTX); //set frequency to some custom frequency
//Auto Transmission Control - dials down transmit power to save battery (-100 is the noise floor, -90 is still pretty good)
//For indoor nodes that are pretty static and at pretty stable temperatures (like a MotionMote) -90dBm is quite safe
//For more variable nodes that can expect to move or experience larger temp drifts a lower margin like -70 to -80 would probably be better
//Always test your ATC mote in the edge cases in your own environment to ensure ATC will perform as you expect
#ifdef ENABLE_ATC
  radio.enableAutoPower(ATC_RSSI);
#endif

  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);

  if (flash.initialize())
  {
    Serial.print("SPI Flash Init OK. Unique MAC = [");
    flash.readUniqueId();
    for (byte i=0;i<8;i++)
    {
      Serial.print(flash.UNIQUEID[i], HEX);
      if (i!=8) Serial.print(':');
    }
    Serial.println(']');
  }
  else
    Serial.println("SPI Flash MEM not found (is chip soldered?)...");

#ifdef ENABLE_ATC
  Serial.println("RFM69_ATC Enabled (Auto Transmission Control)\n");
#endif
  lcd.begin(16, 2);
}

void resetRFM69(){
  digitalWrite(RFM69_RESET, HIGH);
  delay(10);
  digitalWrite(RFM69_RESET, LOW);
  delay(100);
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

long lastPeriod = 0;
void loop() {
  
  lcd.clear();
  /*
  //process any serial input
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    if (input >= 48 && input <= 57) //[0,9]
    {
      TRANSMITPERIOD = 100 * (input-48);
      if (TRANSMITPERIOD == 0) TRANSMITPERIOD = 1000;
      Serial.print("\nChanging delay to ");
      Serial.print(TRANSMITPERIOD);
      Serial.println("ms\n");
      lcd.clear();
      lcd.write("DELAY SET");
      delay(1000);
    }

    if (input == 'r') //d=dump register values
      radio.readAllRegs();
    if (input == 'E') //E=enable encryption
      radio.encrypt(KEY);
    if (input == 'e') //e=disable encryption
      radio.encrypt(null);

    if (input == 'd') //d=dump flash area
    {
      Serial.println("Flash content:");
      uint16_t counter = 0;

      Serial.print("0-256: ");
      while(counter<=256){
        Serial.print(flash.readByte(counter++), HEX);
        Serial.print('.');
      }
      while(flash.busy());
      Serial.println();
    }
    if (input == 'e')
    {
      Serial.print("Erasing Flash chip ... ");
      lcd.clear();
      lcd.write("Erasing Flash chip ... ");
      flash.chipErase();
      while(flash.busy());
      Serial.println("DONE");
    }
    if (input == 'i')
    {
      lcd.clear();
      lcd.write("DeviceID: ");
      Serial.print("DeviceID: ");
      word jedecid = flash.readDeviceId();
      Serial.println(jedecid, HEX);
    }
  }
  */
  //check for any received packets
  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");

    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
      lcd.clear();
      lcd.write(" - ACK sent");
    }
    Blink(LED,3);
    Serial.println();
  }

  int currPeriod = millis()/TRANSMITPERIOD;
  if (currPeriod != lastPeriod)
  {
    lastPeriod=currPeriod;

    //send FLASH id
    if(sendSize==0)
    {
      sprintf(buff, "FLASH_MEM_ID:0x%X", flash.readDeviceId());
      byte buffLen=strlen(buff);
      if (radio.sendWithRetry(GATEWAYID, buff, buffLen))
        Serial.print(" ok!");
      else Serial.print(" nothing...");
      //
      sendSize = (sendSize + 1) % 31;
    }
    else
    {
      Serial.print("Sending[");
      lcd.clear();
      lcd.write("SENDING"); //FREQUENCY
      Serial.print(sendSize);
      Serial.print("]: ");
      for(byte i = 0; i < sendSize; i++)
        Serial.print((char)payload[i]);

      if (radio.sendWithRetry(GATEWAYID, payload, sendSize))
       Serial.print(" ok!");
      else Serial.print(" nothing...");
    }
    sendSize = (sendSize + 1) % 31;
    
    Serial.println();
    //lcd.write(sendSize); //FREQUENCY
    Blink(LED,3);
  }
}

