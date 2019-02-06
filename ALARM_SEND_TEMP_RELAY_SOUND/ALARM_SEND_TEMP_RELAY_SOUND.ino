/*
 * todo:
 *  - remove serial3 "event"
 *  - improve ISR for audio. relation with pin 18, int 5, ex digital 2
 */

#include <Sensirion.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RCSwitch.h>
#include <Wire.h>
#include "RTClib.h"

#define DEBUG_SERIAL
#define ENA_ERRCHK  // Enable error checking code
#define RELAY_TEST
#define DELTA_T           30000 // REACTIVATION TIMER
#define AUDIO_SENSE_DELAY    250

#define ONE_WIRE_BUS     46 //3 //A0 or digital 2....  // Data wire is plugged into pin 2 on the Arduino

#define txPin             4 // digital 4
#define rxPin             0
#define id3Pin           19 // ex 1
#define id2Pin            3 // digital 3

#define button            5 // The button attached to a digital pin
//#define ledPin           13 // Onboard LED = digital pin 13 on arduino uno, 2 on ESP

//choose between 0-5 (2, 3, 18, 19, 20, 21)
//choose between 0-5 (2, 3, 21, 20, 19, 18)
#define IRQ_GATE_IN       5 // IRQ
#define PIN_GATE_IN      18 // gate
#define PIN_ANALOG_IN    A9 // envelope

#define RELAY1           29 //6
#define RELAY2           28 //7
#define RELAY3           27 //7
#define RELAY4           26 //7
#define RELAY5           25 //7
#define RELAY6           24 //7
#define RELAY7           23 //7
#define RELAY8           22 //7
#define RELAY_DELAY     200

#define PIN_LED_OUT      13
/*
int i;
//String reader1[30];
String reader1[32];
volatile int reader1Count = 0;
*/

char daysOfTheWeek[7][12] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
char DateAndTimeString[20]; //19 digits plus the null char

int check_audio_done = 0;
volatile byte state = LOW;

int buttonState = 1;    //Variable to control the flow of code using button presses
int buttonVal   = 0;      //Variable to hold the state of the button
int test_rel    = 0;

volatile long reader1 = 0;
volatile int reader1Count = 0;

const byte gatePin = 18;
const byte dataPin =  7; //23;                 // SHTxx serial data
const byte sclkPin =  6; //32;                 // SHTxx serial clock
const byte ledPin  = 13;                 // Arduino built-in LED

const unsigned long TRHSTEP   = 8000UL;  // Sensor query period
const unsigned long BLINKSTEP =   100UL;  // LED blink period

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

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

unsigned long delayStart = 0; // the time the delay started
bool delayRunning = false; // true if still waiting for delay to finish

RCSwitch mySwitch = RCSwitch();
RCSwitch mySwitchRX = RCSwitch();

RTC_DS3231 rtc;

Sensirion sht = Sensirion(dataPin, sclkPin);
OneWire oneWire(ONE_WIRE_BUS);  // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);  // Pass our oneWire reference to Dallas Temperature.

#ifdef ENA_ERRCHK     // This version of the code checks return codes for errors
byte error = 0;

void setup() {
  byte stat;
  
  myswitch_setup(); delay(500);
  Serial.begin(9600);
  serial_to_esp();
  inputString.reserve(32);

  pinMode(ledPin, OUTPUT); 
  pinMode(PIN_LED_OUT, OUTPUT);
  
  pinMode(button, INPUT);
  //pinMode(PIN_GATE_IN, INPUT_PULLUP);
  pinMode(PIN_GATE_IN, INPUT);
  
  relay_setup();
  rtc_setup();
  sensors.begin();
  delay(50);
  
  attachInterrupt(digitalPinToInterrupt(gatePin), soundISR, CHANGE);
  //attachInterrupt(IRQ_GATE_IN, soundISR, CHANGE);
  
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
  
  setup_wiegand();

  delayStart = millis();   // start delay
  delayRunning = true; // not finished yet
  
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
  master_sequence();
}

#else  // If ENA_ERRCHK is not defined. This code is the same as above but without error checking

void setup() {
  byte stat;
  byte error = 0;

  myswitch_setup(); delay(500);
  Serial.begin(9600);
  serial_to_esp();
  inputString.reserve(32);
  
  pinMode(ledPin, OUTPUT); 
  pinMode(PIN_LED_OUT, OUTPUT);
  
  pinMode(button, INPUT);
  //pinMode(PIN_GATE_IN, INPUT_PULLUP);
  pinMode(PIN_GATE_IN, INPUT);

  relay_setup();
  rtc_setup();
  sensors.begin();
  delay(50);

  attachInterrupt(digitalPinToInterrupt(gatePin), soundISR, CHANGE);
  //attachInterrupt(IRQ_GATE_IN, soundISR, CHANGE);
  
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
  
  setup_wiegand();
  
  delayStart = millis();   // start delay
  delayRunning = true; // not finished yet
  
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

  master_sequence();
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
  //Serial3.write((uint8_t *) &temperature,4); //Serial3.write((uint8_t *) &humidity,4); //Serial3.write((uint8_t *) &dewpoint,4);
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
    
    Serial3.print("RFID: ");
    Serial3.println(reader1,HEX);
    Serial3.println(reader1& 0xfffffff);

    /*
    int serialNumber=(reader1 >> 1) & 0x3fff;
    int siteCode= (reader1 >> 17) & 0x3ff;

    Serial3.print("RF:");
    Serial3.print(siteCode);
    Serial3.print("-");
    Serial3.println(serialNumber);
    */
    
    reader1 = 0;
    reader1Count = 0;
    //digitalWrite(13,HIGH);
    delay(25);
    //digitalWrite(13,LOW);
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
    delay(250);
    buttonState=99;
    Serial.println("B:SHORT");
    Serial3.println("B:SHORT");  
  }
  
  buttonVal = digitalRead(button);
  //if(buttonVal==HIGH){
  //while(buttonState=99 && buttonVal==HIGH)
  while(buttonState<1 && buttonVal==HIGH){
    Serial.println("B:LONG");
    Serial3.println("B:LONG");
    buttonState=0;
    delay(250);
  }
  buttonState=1;
}

void setup_wiegand() {
  attachInterrupt(digitalPinToInterrupt(id2Pin), reader1Zero, RISING);//DATA0 to pin 2  // attachInterrupt(id2Pin, reader1Zero, FALLING);      // DATA0 to pin 2
  delay(50);
  attachInterrupt(digitalPinToInterrupt(id3Pin), reader1One, RISING); //DATA1 to pin 3  // attachInterrupt(id3Pin, reader1One, FALLING);       // DATA1 to pin 3
  delay(50);
  
  // the interrupt in the Atmel processor mixses out the first negitave pulse as the inputs are already high, so this gives a pulse to each reader input line to get the interrupts
  // working properly. Then clear out the reader variables. The readers are open collector sitting normally at a one so this is OK
  
  for(int i = 2; i<4; i++){
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH); // enable internal pull up causing a one
    digitalWrite(i, LOW); // disable internal pull up causing zero and thus an interrupt
    pinMode(i, INPUT);
    digitalWrite(i, HIGH); // enable internal pull up
  }
  
  delay(100);
  // put the reader input variables to zero
  reader1 = 0;
  reader1Count = 0;
  
  /*
  for (i = 0; i < 30; i = i + 1) {
    //  reader1[i] = 0;
  }
  */
}

void soundISR() {
  // this function is installed as an interrupt service routine for the pin change interrupt.
  // When digital input 2 changes state this routine is called. It queries the state of that pin, sets LED to reflect pin's state.
  volatile int pin_val;
  pin_val = digitalRead(PIN_GATE_IN);
  //digitalWrite(PIN_LED_OUT, pin_val);
  
  audio_relay(pin_val,2);
  //check_audio_done = 99;
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
  mySwitch.enableTransmit(txPin);         // Transmitter is connected to Arduino Pin #10  // #4
  mySwitchRX.enableReceive(rxPin);          // Receiver on interrupt 0 => that is pin #2
  // mySwitch.setProtocol(2);             // Optional set protocol (default is 1, will work for most outlets)
  mySwitch.setPulseLength(360);           // Optional set pulse length.
  mySwitch.setRepeatTransmit(5);          // Optional set number of transmission repetitions.
  //Serial3.write(mystr, 5);
}

void relay_setup() {
  pinMode(RELAY1, OUTPUT); delay(RELAY_DELAY);
  pinMode(RELAY2, OUTPUT); delay(RELAY_DELAY);
  pinMode(RELAY3, OUTPUT); delay(RELAY_DELAY);
  pinMode(RELAY4, OUTPUT); delay(RELAY_DELAY);
  pinMode(RELAY5, OUTPUT); delay(RELAY_DELAY);
  pinMode(RELAY6, OUTPUT); delay(RELAY_DELAY);
  pinMode(RELAY7, OUTPUT); delay(RELAY_DELAY);
  pinMode(RELAY8, OUTPUT); delay(RELAY_DELAY*3);

#ifdef RELAY_TEST
    digitalWrite(RELAY1,1);    delay(RELAY_DELAY);
    digitalWrite(RELAY2,1);    delay(RELAY_DELAY);
    digitalWrite(RELAY3,1);    delay(RELAY_DELAY);
    digitalWrite(RELAY4,1);    delay(RELAY_DELAY);
    digitalWrite(RELAY5,1);    delay(RELAY_DELAY);
    digitalWrite(RELAY6,1);    delay(RELAY_DELAY);
    digitalWrite(RELAY7,1);    delay(RELAY_DELAY);
    digitalWrite(RELAY8,1);    delay(RELAY_DELAY);
#endif
}

void rc_alarm_listen() {
  char strxxx[120];
  unsigned long valuexxx;
  int status=0;
  if (mySwitchRX.available()) {
    //output(mySwitchRX.getReceivedValue(), mySwitchRX.getReceivedBitlength(), mySwitchRX.getReceivedDelay(), mySwitchRX.getReceivedRawdata(),mySwitchRX.getReceivedProtocol());
    //process_rf_value(mySwitchRX,434); // does resetavailable
    
    if (mySwitchRX.getReceivedValue() == 13026482 || mySwitchRX.getReceivedValue() == 1979954) {
      Serial.println("ALARM OFF");
      Serial3.println("ALARM OFF");
      status=0;      
    } else if (mySwitchRX.getReceivedValue() == 13026481 || mySwitchRX.getReceivedValue() == 1979953) {
      Serial.println("ALARM ON");
      Serial3.println("ALARM ON");
      status=1;
    } else if (mySwitchRX.getReceivedValue() == 15116296 ) {
      Serial.println("MSENSE:1");
      Serial3.println("MSENSE:1");
      status=2;      
    } else if (mySwitchRX.getReceivedValue() == 10752008 ) {
      Serial.println("MSENSE:2");
      Serial3.println("MSENSE:2");
      status=2;
    } else if (mySwitchRX.getReceivedValue() == 1400576 ) {
      Serial.println("MSENSE:BW");
      Serial3.println("MSENSE:BW");
      status=2;
    } else if (mySwitchRX.getReceivedValue() == 1400576 ) {
      Serial.println("MSENSE:BT");
      Serial3.println("MSENSE:BT");
      status=2;
    }
    
    // upstairs 13026482, downstairs    1979954
    // movement-1 15116296, movement-2 10752008, movement-BW 1400576
    // doorbell:       1400581   // dog effect
    //                 1400592   // knock knock
    //                 1400656   // christmas
    //                 1400660   // fur elisa
    //                 1400580   // mexico
    // door sensor:    1400576   // doorbell sound, depending on ringtone !

    /*
    valuexxx = mySwitchRX.getReceivedValue();
    sprintf(strxxx, "%s / %010lu / %02d bit / Protocol = %d", tobin32(valuexxx), valuexxx, mySwitchRX.getReceivedBitlength(), mySwitchRX.getReceivedProtocol() );
    //Serial.print("YO YO:");
    //Serial.println(strxxx);
    */
    
    mySwitchRX.resetAvailable();  // already done in "process_rf_value"
    //delay(25);
    #ifdef DEBUG_SERIAL
      blinker();
    #endif
  }
}

void rc_alarm_activate() {
  mySwitch.setProtocol(1);
  mySwitch.setPulseLength(360);           // Optional set pulse length.
  
  // upper room on 001110100100011110110001
  mySwitch.send("001110100100011110110010");
  delay(250);
  //groundfloor on 000111100011011000110011
  mySwitch.send("000111100011011000110010");
  delay(250);  
  //first floor on 110001101100010010110001
  mySwitch.send("110001101100010010110010");
  delay(250);
  
  //dog
/*
  mySwitch.setProtocol(2);
  mySwitch.setPulseLength(220);           // Optional set pulse length
  mySwitch.setRepeatTransmit(2);          // Optional set number of transmission repetitions.
*/
  //Serial.println(tobin32(1400581));
  //mySwitch.send("000101010101111100000101");
  mySwitch.send(1400581,24); // dog
  //mySwitch.send(15116296,24); // movement 1
  delay(250);
  mySwitch.sendTriState("0FFFFF1100FF"); // dog
  delay(250);
  
  // 2 rcv
  // 101001000001000000001000 // pulse 200
  // 1 rcv
  // 111001101010100000001000 // pulse 200
  // button
  // 000101010101111100000101 // tristate 0FFFFF1100FF // pulse 220
  // black & white rcv
  // 000101010101111100000000 // tristate 0FFFFF110000 // pulse 380

  #ifdef DEBUG_SERIAL
    Serial.println("ALARM-OVERRIDE:ON");
    Serial3.println("ALARM-OVERRIDE:ON");
  #endif
}

void query_dallas_sensors() {
  //delay(50);
  // call sensors.requestTemperatures() to issue a global temperature request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures
  delay(50);
  
  Serial3.print("X:");
  Serial3.print(sensors.getTempCByIndex(0)); // Why "byIndex"?
  Serial3.println("");
  Serial.print("X:");
  Serial.print(sensors.getTempCByIndex(0)); // Why "byIndex"?
  Serial.println("");
}

void serial_to_esp(){
  Serial3.begin(9600);  
}

void serial_write_esp() {
  Serial3.write(mystr, 512);
  delay(500);
}

void rtc_print() {
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
    delay(500);
    while (1);
  }
  
  //Sets the code compilation time to RTC DS3231
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void serial_read() {
  while (Serial3.available() > 0 ) {
    if(indexxx < 8) {
      inChar = Serial3.read();                // Read a character
      inData[indexxx] = inChar;               // Store it
      indexxx++;                              // Increment where to write next
      inData[indexxx] = '\0';                 // Null terminate the string
    } else {
      indexxx = 0;
      Serial3.print("COMM:");
      //Serial3.println(digitalRead(ledPin));
      Serial3.print(inData[0]);
      Serial3.print(inData[1]);
      Serial3.print(inData[2]);
      Serial3.print(inData[4]);
      Serial3.print(inData[5]);
      Serial3.print(inData[6]);
      Serial3.println(inData[7]);
    }
    //indexxx = 0;
  }
}

//void serialEvent1, void serialEvent2, void serialEvent3
void serialEvent3() {
  //while (Serial3.available()) {
  while(stringComplete==false){
    char inChar = (char)Serial3.read();
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
      Serial3.print("TST:");
      Serial3.println(inputString);
      //Serial.print("TST:");
      //Serial.println(inputString);
    
    }
  }
}

void audio_relay(int test, byte range) {
  /*
  if (range == 2) {
    Serial.print("P:test-audio_relay:");
    Serial.print(test);
    Serial.print(" - ");
    Serial.println(range);
    Serial3.print("P:test-audio_relay:");
    Serial3.print(test);
    Serial3.print(" - ");
    Serial3.println(range);
  }
  */
  //check_audio_done = test;
  
  if(check_audio_done <= 1) {
    delay(25);
  } else if(check_audio_done == 2) {
    delay(AUDIO_SENSE_DELAY);
    digitalWrite(RELAY1,0);
    delay(AUDIO_SENSE_DELAY);
    check_audio_done = 2;
  } else if(check_audio_done == 3) {
    delay(AUDIO_SENSE_DELAY);
    digitalWrite(RELAY2,0);
    delay(AUDIO_SENSE_DELAY);
    check_audio_done = 3;
  } else if(check_audio_done == 4) {
    delay(AUDIO_SENSE_DELAY);
    digitalWrite(RELAY8,0);
    delay(AUDIO_SENSE_DELAY);
    check_audio_done = 4;
  } else if(check_audio_done == 99) {
    delay(AUDIO_SENSE_DELAY);
    digitalWrite(RELAY8,0);
    delay(AUDIO_SENSE_DELAY);
    digitalWrite(RELAY2,0);
    delay(AUDIO_SENSE_DELAY);
    digitalWrite(RELAY1,0);
    delay(AUDIO_SENSE_DELAY);
    check_audio_done = 99;
    Serial.println("AUDIO:99");
    Serial3.println("AUDIO:99");
  }
  
  //check_audio_in();
  //check_audio_done = 0;
}

// half a second routine
void timer_reset() {
  if (delayRunning && ((millis() - delayStart) >= DELTA_T)) {
    rtc_print();
    delayRunning = false; // // prevent this code being run more then once
    rc_alarm_activate();
    query_dallas_sensors();
    digitalWrite(RELAY1,1);    delay(RELAY_DELAY);
    digitalWrite(RELAY2,1);    delay(RELAY_DELAY);
    digitalWrite(RELAY8,1);    delay(RELAY_DELAY);
    timer_restart();
  }
}

void check_audio_in() {
  int value;
  value = analogRead(PIN_ANALOG_IN);  // Check the envelope input
  
  delay(50);
  //Serial.println(value);
  //if(value > 20) Serial3.println(value);
  
  if(value <= 10) {
    //Serial.println("Quiet.");
    //Serial3.write("QUIET   ", 8);
    //Serial3.println("QUIET   ");
    //Serial3.println(value);
    check_audio_done = 0;
  } else if( (value > 10) && ( value <= 20) ) {
    //Serial.println("Something.");
    //Serial3.write("SOME    ", 8);
    //Serial3.println("SOME    ");
    //Serial3.println(value);
    check_audio_done = 1;
  } else if( (value > 20) && ( value <= 70) ) {
    Serial.println("Person.");
    //Serial3.write("PERSON  ", 8);
    Serial3.println("PERSON  ");
    //Serial3.println(value);
    check_audio_done = 2;
  } else if( (value > 70) && ( value <= 140) ) {
    Serial.println("Multiple persons.");
    //Serial3.write("PEOPLE  ", 8);
    Serial3.println("PEOPLE  ");
    //Serial3.println(value);
    check_audio_done = 3;
  } else if(value > 140) {
    Serial.println("Loud, party, intrusion.");
    //Serial3.write("ALARM   ", 8);
    Serial3.println("ALARM   ");
    //Serial3.println(value);
    check_audio_done = 4;
  }
  //delayStart = millis();   // start delay
  //delayRunning = true; // not finished yet
  //check_audio_done = 0;
  delay(50);
}

void timer_restart() {
  check_audio_done = 0;
  delayStart = millis();   // start delay
  delayRunning = true; // not finished yet
}

void blinker() {
  for (int q=0; q<10; q=q+1){
    digitalWrite(ledPin, HIGH);
    delay(25);
    digitalWrite(ledPin,LOW);
    delay(25);
  }
}

void master_sequence() {
  rc_alarm_listen();
  read_button_in();
  //delay(25);
  check_audio_in();
  delay(50);
  audio_relay(0,1);
  timer_reset();
  check_wiegand_in();
  
  serial_read();
  //serial_write_esp();
  delay(25);
}

// simple decimal-to-binary-ascii procedure
char *tobin32(unsigned long x) {
  static char b[33];
  b[32] = '\0';
  for ( int z = 0; z < 32; z++) {
    b[31 - z] = ((x >> z) & 0x1) ? '1' : '0';
  }
  return b;
}
 
void process_rf_value(RCSwitch rfswitch, int rf) {
  char str[120];
  unsigned long value;
  
  //digitalWrite(PIN_LED_OUT, true);
  value = rfswitch.getReceivedValue();
  
  if (value) {
    sprintf(str, "[+] %d Received: %s / %010lu / %02d bit / Protocol = %d",
      rf, tobin32(value), value, rfswitch.getReceivedBitlength(), rfswitch.getReceivedProtocol() );
  } else {
    sprintf(str, "[-] %d Received: Unknown encoding (0)", rf);
  }

  Serial.println(str);
  Serial3.println(str);

  //rfswitch.send(value, rfswitch.getReceivedBitlength());
  //digitalWrite(PIN_LED_OUT, false);
  rfswitch.resetAvailable();
}
