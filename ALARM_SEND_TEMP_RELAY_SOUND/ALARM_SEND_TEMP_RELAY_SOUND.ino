
#include <Sensirion.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RCSwitch.h>
#include <Wire.h>
#include "RTClib.h"

#define ENA_ERRCHK  // Enable error checking code

#define ONE_WIRE_BUS  48 //3 //A0 or digital 2....  // Data wire is plugged into pin 2 on the Arduino
#define txPin         4
#define rxPin         2 // pin 18, choose between 0-5 (2, 3, 18, 19, 20, 21)
#define id3Pin        1 
#define id2Pin        3 
#define button        5         //The button attached to a digital pin
//#define ledPin 13        //Onboard LED = digital pin 13
#define PIN_GATE_IN   2   // gate
#define IRQ_GATE_IN   0
#define PIN_LED_OUT  13
#define PIN_ANALOG_IN A15 //A0 // envelope

#define RELAY1       22 //6
#define RELAY2       23 //7
#define RELAY3       24 //7
#define RELAY4       25 //7
#define RELAY5       26 //7
#define RELAY6       27 //7
#define RELAY7       28 //7
#define RELAY8       29 //7

/*
int i;
//String reader1[30];
String reader1[32];
volatile int reader1Count = 0;
*/

char daysOfTheWeek[7][12] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
char DateAndTimeString[20]; //19 digits plus the null char

int buttonState = 1;    //Variable to control the flow of code using button presses
int buttonVal = 0;      //Variable to hold the state of the button
int test_rel = 0;

volatile long reader1 = 0;
volatile int reader1Count = 0;

const byte dataPin =  7; //23;                 // SHTxx serial data
const byte sclkPin =  6; //32;                 // SHTxx serial clock
const byte ledPin  = 13;                 // Arduino built-in LED

const unsigned long TRHSTEP   = 10000UL;  // Sensor query period
const unsigned long BLINKSTEP =   250UL;  // LED blink period

unsigned int rawData;                   // ARDUINO
//uint32_t rawData;                     // ESP8266 unsigned int = uint32_t;

float temperature;
float humidity;
float dewpoint;

byte ledState = 0;
byte measActive = false;
byte measType = TEMP;

unsigned long trhMillis = 0;             // Time interval tracking
unsigned long blinkMillis = 0;

char mystr[512] = "";
char inData[32]; // Allocate some space for the string
char inChar=-1; // Where to store the character read
byte indexxx = 0; // Index into array; where to store the character

RCSwitch mySwitchRX = RCSwitch();
RCSwitch mySwitch = RCSwitch();
RTC_DS3231 rtc;
OneWire oneWire(ONE_WIRE_BUS);  // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);  // Pass our oneWire reference to Dallas Temperature.
Sensirion sht = Sensirion(dataPin, sclkPin);



#ifdef ENA_ERRCHK     // This version of the code checks return codes for errors
byte error = 0;

void setup() {
  
  byte stat;
  Serial.begin(9600);
  serial_to_esp();
  
  pinMode(ledPin, OUTPUT); 
  pinMode(button, INPUT);
  pinMode(PIN_LED_OUT, OUTPUT);
  pinMode(PIN_GATE_IN, INPUT);

  relay_setup();
  rtc_setup();
  sensors.begin();
  delay(15);                             // Wait >= 11 ms before first cmd
  
  attachInterrupt(IRQ_GATE_IN, soundISR, CHANGE);
  myswitch_setup();
  
  // Demonstrate status register read/write
  if (error = sht.readSR(&stat))         // Read sensor status register
    logError(error);
  Serial3.print("SREG:0x");
  Serial3.println(stat, HEX);
  //if (error = sht.writeSR(HIGH_RES))     // Set sensor to high resolution * new
  if (error = sht.writeSR(LOW_RES))      // Set sensor to low resolution
    logError(error);
  if (error = sht.readSR(&stat))         // Read sensor status register again
    logError(error);
  Serial3.print("SREG:0x");
  Serial3.println(stat, HEX);

  // Demonstrate blocking calls
  if (error = sht.measTemp(&rawData))    // sht.meas(TEMP, &rawData, BLOCK)
    logError(error);
  temperature = sht.calcTemp(rawData); // ARDUINO
  if (error = sht.measHumi(&rawData))    // sht.meas(HUMI, &rawData, BLOCK)
    logError(error);
  humidity = sht.calcHumi(rawData, temperature); // ARDUINO
  dewpoint = sht.calcDewpoint(humidity, temperature);
  logData();
  
  set_reader_up();
  
}

void loop() {
  unsigned long curMillis = millis();          // Get current time

  // Rapidly blink LED.  Blocking calls take too long to allow this.
  if (curMillis - blinkMillis >= BLINKSTEP) {  // Time to toggle the LED state?
    ledState ^= 1;
    digitalWrite(ledPin, ledState);
    blinkMillis = curMillis;
  }

  // Demonstrate non-blocking calls
  if (curMillis - trhMillis >= TRHSTEP) {      // Time for new measurements?
    measActive = true;
    measType = TEMP;
    if (error = sht.meas(TEMP, &rawData, NONBLOCK)) // Start temp measurement
      logError(error);
    trhMillis = curMillis;
  }
  
  if (measActive && (error = sht.measRdy())) { // Check measurement status
    if (error != S_Meas_Rdy)
      logError(error);
    if (measType == TEMP) {                    // Process temp or humi?
      measType = HUMI;
      temperature = sht.calcTemp(rawData);     // Convert raw sensor data ARDUINO
      if (error = sht.meas(HUMI, &rawData, NONBLOCK)) // Start humi measurement
        logError(error);
    } else {
      measActive = false;
      humidity = sht.calcHumi(rawData, temperature); // Convert raw sensor data ARDUINO
      dewpoint = sht.calcDewpoint(humidity, temperature);
      logData();
    }
  }

  //read_button_in();
  query_sensors();
  activate_rc_alarm();
  check_audio_in();
  //delay(500);
  //serial_write_esp();
  
  if (mySwitchRX.available()) {
    output(mySwitchRX.getReceivedValue(), mySwitchRX.getReceivedBitlength(), mySwitchRX.getReceivedDelay(), mySwitchRX.getReceivedRawdata(),mySwitchRX.getReceivedProtocol());
    mySwitchRX.resetAvailable();
  }
  
  /*
  if (reader1Count >= 31) {
    //Serial.print("Reader 1 ");
    //Serial.println(reader1,HEX);
    //for (i = 0; i < 26; i = i + 1)
    
    for (i = 0; i < 31; i = i + 1) {
      Serial.print(reader1[i]);
      //  reader1[i] = 0;
    }
    Serial.println("");
    reader1Count = 0;
  }
  */

  rtc_display() ;
  check_wiegand_in();
  serial_read();
}

#else  // If ENA_ERRCHK is not defined. This code is the same as above but without error checking

void setup() {
  byte stat;
  byte error = 0;
  Serial.begin(9600);
  serial_to_esp();
  
  pinMode(ledPin, OUTPUT); 
  pinMode(button, INPUT);
  pinMode(PIN_LED_OUT, OUTPUT);
  pinMode(PIN_GATE_IN, INPUT);
  
  relay_setup();
  rtc_setup();
  sensors.begin();
  delay(15);                             // Wait >= 11 ms before first cmd

  attachInterrupt(IRQ_GATE_IN, soundISR, CHANGE);
  myswitch_setup();
  
  // Demonstrate status register read/write
  sht.readSR(&stat);                     // Read sensor status register
  Serial3.print("SREG:0x");
  Serial3.println(stat, HEX);
  //sht.writeSR(LOW_RES);                  // Set sensor to low resolution
  //sht.writeSR(HIGH_RES);                  // Set sensor to low resolution * new
  
  sht.readSR(&stat);                     // Read sensor status register again
  Serial3.print("SREG:0x");
  Serial3.println(stat, HEX);

  // Demonstrate blocking calls
  sht.measTemp(&rawData);                // sht.meas(TEMP, &rawData, BLOCK)
  temperature = sht.calcTemp(rawData);
  sht.measHumi(&rawData);                // sht.meas(HUMI, &rawData, BLOCK)
  humidity = sht.calcHumi(rawData, temperature);
  dewpoint = sht.calcDewpoint(humidity, temperature);
  logData();
  
  set_reader_up();
}

void loop() {
  unsigned long curMillis = millis();          // Get current time
  // Rapidly blink LED.  Blocking calls take too long to allow this.
  if (curMillis - blinkMillis >= BLINKSTEP) {  // Time to toggle the LED state?
    ledState ^= 1;
    digitalWrite(ledPin, ledState);
    blinkMillis = curMillis;
  }

  // Demonstrate non-blocking calls
  if (curMillis - trhMillis >= TRHSTEP) {      // Time for new measurements?
    measActive = true;
    measType = TEMP;
    sht.meas(TEMP, &rawData, NONBLOCK);        // Start temp measurement
    trhMillis = curMillis;
  }
  
  if (measActive && sht.measRdy()) {           // Check measurement status
    if (measType == TEMP) {                    // Process temp or humi?
      measType = HUMI;
      temperature = sht.calcTemp(rawData);     // Convert raw sensor data
      sht.meas(HUMI, &rawData, NONBLOCK);      // Start humi measurement
    } else {
      measActive = false;
      humidity = sht.calcHumi(rawData, temperature); // Convert raw sensor data
      dewpoint = sht.calcDewpoint(humidity, temperature);
      logData();
    }
  }

  //read_button_in();
  query_sensors();
  activate_rc_alarm();
  check_audio_in();
  serial_write_esp();
   
  if (mySwitchRX.available()) {
    output(mySwitchRX.getReceivedValue(), mySwitchRX.getReceivedBitlength(), mySwitchRX.getReceivedDelay(), mySwitchRX.getReceivedRawdata(),mySwitchRX.getReceivedProtocol());
    mySwitchRX.resetAvailable();
  }

  rtc_display():
  check_wiegand_in();
  serial_read();
}

#endif  // End of non-error-checking example

void logData() {
  Serial3.print("T:");  Serial3.println(temperature);
  Serial3.print("H:");  Serial3.println(humidity);
  Serial3.print("D:");  Serial3.println(dewpoint);
  //Serial3.println(" °C");
  delay(500);
  //float serial_temp = sensors.getTempCByIndex(0);
  Serial.print("T:");  Serial.println(temperature);
  Serial.print("H:");  Serial.println(humidity);
  Serial.print("D:");  Serial.println(dewpoint);
  
  //Serial3.write((uint8_t *) &temperature,4);
  //Serial3.write((uint8_t *) &humidity,4);
  //Serial3.write((uint8_t *) &dewpoint,4);
}

// The following code is only used with error checking enabled
void logError(byte error) {
  switch (error) {
    case S_Err_NoACK:
      //Serial3.println("Error: No response (ACK) received from sensor!");
      Serial3.println("E:NACK  ");
      break;
    case S_Err_CRC:
      //Serial3.println("Error: CRC mismatch!");
      Serial3.println("E:CRCM  ");
      break;
    case S_Err_TO:
      Serial3.println("E:TOUT  ");
      break;
    default:
      Serial3.println("E:UNK   ");
      break;
  }
}

void check_wiegand_in() {
  if(reader1Count >=26){
    //Serial.print("Reader 1 ");
    //Serial.println(reader1,HEX);
    //Serial.println(reader1& 0xfffffff);
    int serialNumber=(reader1 >> 1) & 0x3fff;
    int siteCode= (reader1 >> 17) & 0x3ff;

    Serial3.print("RF:");
    Serial3.print(siteCode);
    Serial3.print("-");
    Serial3.println(serialNumber);
    reader1 = 0;
    reader1Count = 0;
    digitalWrite(13,HIGH);
    delay(500);
    digitalWrite(13,LOW);
  }
  
  /*
  if (reader1Count >= 31) {
    //Serial.print("Reader 1 ");
    //Serial.println(reader1,HEX);
    //for (i = 0; i < 26; i = i + 1)
    
    for (i = 0; i < 31; i = i + 1) {
      Serial.print(reader1[i]);
      //  reader1[i] = 0;
    }
    Serial.println("");
    reader1Count = 0;
  }
  */
}

void read_button_in() {
  buttonVal = digitalRead(button);
  if(buttonState>0 && buttonVal==HIGH){
    Serial.println("B:PRESSD");
    Serial3.println("B:PRESSD");
    delay(50);
    buttonState=0;
  }
  
  buttonVal = digitalRead(button);
  if(buttonState<1 && buttonVal==HIGH){
    Serial.println("B:RELSD");
    Serial3.println("B:RELSD");
    buttonState=1;
  }
  delay(20);
}

void check_audio_in() {
  int value;
  value = analogRead(PIN_ANALOG_IN);  // Check the envelope input
  
  if(value <= 10) {
    //Serial.println("Quiet.");
    //Serial3.write("QUIET   ", 8);
    //Serial3.println("QUIET   ");
  } else if( (value > 10) && ( value <= 20) ) {
    Serial.println("Something.");
    //Serial3.write("SOME    ", 8);
    Serial3.println("SOME    ");
  } else if( (value > 20) && ( value <= 70) ) {
    Serial.println("Person.");
    //Serial3.write("PERSON  ", 8);
    Serial3.println("PERSON  ");
  } else if( (value > 70) && ( value <= 140) ) {
    Serial.println("Multiple persons.");
    //Serial3.write("PEOPLE  ", 8);
    Serial3.println("PEOPLE  ");
  } else if(value > 140) {
    Serial.println("Loud, party, intrusion.");
    //Serial3.write("ALARM   ", 8);
    Serial3.println("ALARM   ");
  }
}

void set_reader_up() {
  /*
  attachInterrupt(id2Pin, reader1Zero, FALLING);      // DATA0 to pin 2
  attachInterrupt(id3Pin, reader1One, FALLING);       // DATA1 to pin 3
  delay(10);

  // put the reader input variables to zero
  for (i = 0; i < 30; i = i + 1) {
    //  reader1[i] = 0;
  }
  //reader1 = 0;              // put the reader input variables to zero
  reader1Count = 0;
  */

  attachInterrupt(id2Pin, reader1Zero, RISING);//DATA0 to pin 2
  attachInterrupt(id3Pin, reader1One, RISING); //DATA1 to pin 3
  delay(10);
  // the interrupt in the Atmel processor mixses out the first negitave pulse as the inputs are already high, so this gives a pulse to each reader input line to get the interrupts
  // working properly. Then clear out the reader variables. The readers are open collector sitting normally at a one so this is OK
  
  for(int i = 2; i<4; i++){
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH); // enable internal pull up causing a one
    digitalWrite(i, LOW); // disable internal pull up causing zero and thus an interrupt
    pinMode(i, INPUT);
    digitalWrite(i, HIGH); // enable internal pull up
  }
  
  delay(10);
  // put the reader input variables to zero
  reader1 = 0;
  reader1Count = 0;
}

void soundISR() {
  // this function is installed as an interrupt service routine for the pin change interrupt.  When digital input 2 changes state, this routine is called. It queries the state of that pin, and sets the onboard LED to reflect that pin's state.
  int pin_val;
  pin_val = digitalRead(PIN_GATE_IN);
  digitalWrite(PIN_LED_OUT, pin_val);
}

/*
void reader1One(void) {
  reader1[reader1Count] = "1";
  reader1Count++;
}

void reader1Zero(void) {
  reader1[reader1Count] = "0";
  reader1Count++;
}
*/

void reader1One(void) {
  reader1Count++;
  reader1 = reader1 << 1;
  reader1 |= 1;
}

void reader1Zero(void) {
  reader1Count++;
  reader1 = reader1 << 1;  
}

void myswitch_setup() {
  // Transmitter is connected to Arduino Pin #10  // #4
  mySwitch.enableTransmit(txPin);
  // Optional set protocol (default is 1, will work for most outlets)
  // mySwitch.setProtocol(2);
  // Optional set pulse length.
  mySwitch.setPulseLength(360);
  // Optional set number of transmission repetitions.
  mySwitch.setRepeatTransmit(5);

  mySwitchRX.enableReceive(rxPin);  // Receiver on interrupt 0 => that is pin #2
  //Serial3.write(mystr, 5);
}

void relay_setup() {
  pinMode(RELAY1, OUTPUT); delay(100);
  pinMode(RELAY2, OUTPUT); delay(100);
  pinMode(RELAY3, OUTPUT); delay(100);
  pinMode(RELAY4, OUTPUT); delay(100);
  pinMode(RELAY5, OUTPUT); delay(100);
  pinMode(RELAY6, OUTPUT); delay(100);
  pinMode(RELAY7, OUTPUT); delay(100);
  pinMode(RELAY8, OUTPUT); delay(100);

  //if (test_rel=0) {
    digitalWrite(RELAY1,0);    delay(100);
    digitalWrite(RELAY2,0);    delay(100);
    digitalWrite(RELAY3,0);    delay(100);
    digitalWrite(RELAY4,0);    delay(100);
    digitalWrite(RELAY5,0);    delay(100);
    digitalWrite(RELAY6,0);    delay(100);
    digitalWrite(RELAY7,0);    delay(100);
    digitalWrite(RELAY8,0);    delay(100);
    
    digitalWrite(RELAY1,1);    delay(100);
    digitalWrite(RELAY2,1);    delay(100);
    digitalWrite(RELAY3,1);    delay(100);
    digitalWrite(RELAY4,1);    delay(100);
    digitalWrite(RELAY5,1);    delay(100);
    digitalWrite(RELAY6,1);    delay(100);
    digitalWrite(RELAY7,1);    delay(100);
    digitalWrite(RELAY8,1);    delay(100);
  //  test_rel=1;
  //}
}

void activate_rc_alarm() {
// upper room on 001110100100011110110001
  mySwitch.send("001110100100011110110010");
  delay(250);
//groundfloor on 000111100011011000110011
  mySwitch.send("000111100011011000110010");
  delay(250);  
//first floor on 110001101100010010110001
  mySwitch.send("110001101100010010110010");
  delay(250);
// 2 rcv
  // 101001000001000000001000 // pulse 200
// 1 rcv
  // 111001101010100000001000 // pulse 200
// button
  // 000101010101111100000101 // tristate 0FFFFF1100FF // pulse 220
// black & white rcv
  // 000101010101111100000000 // tristate 0FFFFF110000 // pulse 380
}

void query_sensors() {
  delay(200);
  // call sensors.requestTemperatures() to issue a global temperature request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures
  delay(50);
  Serial3.print("X:");
  Serial3.print(sensors.getTempCByIndex(0)); // Why "byIndex"?
  //Serial3.println(" °C");
  Serial3.println("");
  //Serial.print(sensors.getTempCByIndex(1))
  // You can have more than one IC on the same bus, 0 refers to the first IC on the wire
}

void serial_to_esp(){
  Serial3.begin(9600);  
}

void serial_write_esp() {
  Serial3.write(mystr, 512);
  delay(500);
}

void rtc_display() {
  DateTime now = rtc.now(); //Set now as RTC time
  //RtcDateTime now = Rtc.GetDateTime();
  /*
  //Print RTC time to Serial Monitor
  Serial.println(now.year(), DEC);  Serial.print('/');  Serial.print(now.month(), DEC);  Serial.print('/');  Serial.print(now.day(), DEC);  Serial.print(" (");  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.println(now.hour(), DEC);  Serial.print(':');  Serial.print(now.minute(), DEC);  Serial.print(':');  Serial.println(now.second(), DEC);  
  */
  // no daysOfTheWeek[now.dayOfTheWeek()]
  sprintf_P(DateAndTimeString, PSTR("%4d-%02d-%02d %02d:%02d:%02d"), now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

  Serial.println(DateAndTimeString);
  Serial3.println(DateAndTimeString);
  //2018-11-13 00:29:26
  {String thisSecond("0" + now.second());}
  String thisSecond = String(now.second(), DEC);
  //Put all the time and date strings into one String
  //DateAndTimeString = String(thisYear + "-" + thisMonth + "-" + thisDay + " " + thisHour + ":" + thisMinute + ":" + thisSecond);
  
  /*
  char buffer [16];
  uint8_t sec, min, hour;
  sec = now.second();
  min = now.minute();
  hour = now.hour();
  sprintf (buffer, "Time: %02u:%02u:%02u\n", hour, min, sec);
  Serial.println(buffer);
  */

}

void rtc_setup() {
  if (! rtc.begin()) {
    Serial3.println("E:NORTC ");
    Serial.println("E:NORTC ");
    while (1);
  }
  
  //Sets the code compilation time to RTC DS3231
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void serial_read() {
  //while (Serial3.available() > 0 ) {
    if(indexxx < 31) {
        inChar = Serial3.read();                // Read a character
        inData[indexxx] = inChar;               // Store it
        indexxx++;                              // Increment where to write next
        inData[indexxx] = '\0';                 // Null terminate the string
    }
    Serial3.print("LED:");
    Serial3.println(digitalRead(ledPin));
    Serial3.println(inChar);
    Serial3.println("CALLBACK");
    indexxx = 0;
  //}
}
