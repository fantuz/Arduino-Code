
#include <RFM69.h>
#include <RFM69_ATC.h>     //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <SPIFlash.h>      //get it here: https://www.github.com/lowpowerlab/spiflash
#include "RF24.h"
#include <IRremote.h>
#include <Wire.h>
#include "RTClib.h"

#define IS_RFM69HCW false
#define NODEID      2
#define NETWORKID   100
#define GATEWAYID   1
#define FREQUENCY   RF69_433MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define KEY         "thisIsEncryptKey" //has to be same 16 characters/bytes on all nodes, not more not less!
#define LED         6
#define SERIAL_BAUD 9600
#define ACK_TIME    500  // # of ms to wait for an ack
  
#define RF69_IRQ_PIN        3 //3 // INT1 on AVRs should be connected to DIO0 (ex on Atmega328 it's D3)

#define SS                  8 //9
#define FLASH_SS            8 //9 //10 // and FLASH SS on D8
#define SPI_CS              9 //10 // SS is the SPI slave select pin, for WireLess Gate Shield - D9
#define RFM69_CS            9
#define RFM69_DIO0          3 //3
#define RFM69_IRQ           3
#define RFM69_NSS           9 //9
#define RFM69_IRQN          1 // Pin 2 is IRQ 0!
#define RFM69_RST           14 //7 
#define RFM69_RESET         14 //7  //A0

#define BUTTON 15 // A1
//#define IS_RFM69HW_HCW  //uncomment only for RFM69HW/HCW! Leave out if you have RFM69W/CW!

int TRANSMITPERIOD = 30; //transmit a packet to gateway so often (in ms)
byte sendSize = 0;
boolean requestACK = true;

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);
SPIFlash flash(FLASH_SS, 0x0000); //EF30 for 4mbit  Windbond chip (W25X40CL)

bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

int delta=2000;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
unsigned long blinkStop;
unsigned long timeReady;
 
RF24 radio24(7,8); 
RTC_DS1307 RTC;
int RECV_PIN = 5;
IRrecv irrecv(RECV_PIN);
decode_results results;
 
// create a framework for the transmission of values
typedef struct{         
  int SensorID;        // ID sensor
  int CommandTo;       // command module number ...
  int Command;         // command
  // 0 - answer 
  // 1 - get the value
  // 2 - set the value
  int ParamID;         // parameter identifier
  float ParamValue;    // value
  boolean Status;      // status
  // 0 - read-only (RO)
  // 1 -  can change the (RW)
  char Comment[16];    // comment
}

Message;
Message sensor; 
 
const uint64_t pipes[2] = { 
  0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
 
volatile boolean waitRF24 = false;

typedef struct {    
  int           nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  float         temp;   //temperature maybe?
} 

Payload;
Payload theData;
 
void setup() {
  Serial.begin(SERIAL_BAUD);
 
  pinMode(LED, OUTPUT);
 
  pinMode(RFM69_NSS, OUTPUT);
  pinMode(RFM69_RESET, OUTPUT);
  pinMode(FLASH_SS, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCK, OUTPUT);
  pinMode(RFM69_RESET, OUTPUT);
  pinMode(RFM69_DIO0, INPUT);
  pinMode(BUTTON, INPUT);
 
  digitalWrite(RFM69_NSS, HIGH);
  digitalWrite(RFM69_RESET, HIGH);
 
  resetRFM69();
  radio.setCS(RFM69_NSS);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
   //radio.setHighPower(); //uncomment only for RFM69HW!
  radio.encrypt(KEY);
  radio.setFrequency(434.0);

  delay(20);

  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
 
  radio24.begin();
  // optionally, increase the delay between retries & # of retries
  radio24.setRetries(15,15);
  radio24.setChannel(119);
  // по умолчанию СЛУШАЕМ
  radio24.openWritingPipe(pipes[1]);
  radio24.openReadingPipe(1,pipes[0]);
  radio24.startListening();
 
  delay(20);
 
  attachInterrupt(0, isr_RF24, FALLING);
 
  irrecv.enableIRIn();
 
  Wire.begin();
  RTC.begin();
 
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  //Serial.println(RTC.getMinutes());
  //DateTime dt7 (unixtime()); // One hour later.
  //showDate(); //"dt7", dt7);
   DateTime now = RTC.now();
    
   Serial.print(now.year(), DEC);
   Serial.print('/');
   Serial.print(now.month(), DEC);
   Serial.print('/');
   Serial.print(now.day(), DEC);
   //Serial.print(" (");
   //Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
   Serial.print(" ");
   //Serial.print(") ");
   Serial.print(now.hour(), DEC);
   Serial.print(':');
   Serial.print(now.minute(), DEC);
   Serial.print(':');
   Serial.print(now.second(), DEC);
   Serial.println();
}
 
long lastPeriod = -1;
/*
void showDate(const char* txt, const DateTime& dt) {
    Serial.print(txt);
    Serial.print(' ');
    Serial.print(dt.year(), DEC);
    Serial.print('/');
    Serial.print(dt.month(), DEC);
    Serial.print('/');
    Serial.print(dt.day(), DEC);
    Serial.print(' ');
    Serial.print(dt.hour(), DEC);
    Serial.print(':');
    Serial.print(dt.minute(), DEC);
    Serial.print(':');
    Serial.print(dt.second(), DEC);
    
    Serial.print(" = ");
    Serial.print(dt.unixtime());
    Serial.print("s / ");
    Serial.print(dt.unixtime() / 86400L);
    Serial.print("d since 1970");
    
    Serial.println();
}
*/
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
  
  //check for any received packets
  if (radio.receiveDone())
  {
    Serial.print('[');
    Serial.print(radio.SENDERID, DEC);
    Serial.print("] ");
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print("   [RX_RSSI:");
    Serial.print(radio.readRSSI());
    Serial.print("]");
 
    if (radio.ACK_REQUESTED)
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
      delay(10);
    }
    Serial.println();
  }
 
  int currPeriod = millis()/TRANSMITPERIOD;
  if (currPeriod != lastPeriod)
  {
    //fill in the struct with new values
    theData.nodeId = NODEID;
    theData.uptime = millis();
    theData.temp = radio.readTemperature();//91.23; //it's hot!
 
    Serial.print("Sending struct (");
    Serial.print(sizeof(theData));
    Serial.print(" bytes) ... ");
    if (radio.sendWithRetry(GATEWAYID, (const void*)(&theData), sizeof(theData)))
      Serial.print(" ok!");
    else Serial.print(" nothing...");
    Serial.println();
    lastPeriod=currPeriod;
  }
 
  listenRF24();
 
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
    blinkStop=millis()+100;
    digitalWrite(LED, HIGH);
  }
 
  if (digitalRead(BUTTON)==LOW) {
    blinkStop=millis()+1000;
    digitalWrite(LED, HIGH);
  }
 
  if (millis()>blinkStop) {
    digitalWrite(LED, LOW);
  }
 
  if(millis()>timeReady){
    timeReady=millis()+2000;
    DateTime now = RTC.now();
 
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
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
 
void isr_RF24(){
  waitRF24 = true;
}

void listenRF24() {
  if (waitRF24) {
    waitRF24 = false;
    if ( radio24.available() )
    {
      bool done = false;
      Serial.print("inside radio loop");
      //bool done = false;
      while (!done)
      //while (radio.available)
      {
        radio24.read(&sensor, sizeof(sensor) );
        //if ((sizeof(radio24.read(&sensor, sizeof(sensor))))>0) {
        //  done = true;
        //}
        if(sensor.Command == 0) {  
          Serial.print(sensor.SensorID);
          Serial.print("  ");
          Serial.print(sensor.ParamID);
          Serial.print("  ");
          Serial.print(sensor.ParamValue);
          Serial.print(" ");
          Serial.println(sensor.Comment);
        }
      }
    }
  }
}

