// Date, Time and Alarm functions using a DS3231 RTC connected via I2C and Wire lib
// The RTC can wake up the Arduino, if the Arduino has been put to sleep (to save power). 
// This can be a handy function for battery operated units.
//
// Hardware: connect RTC SQW/INT pin to Arduino pin2
// during an alarm the RTC SQW/INT pin is pulled low

#include <Wire.h>
#include <avr/sleep.h>
#include <RTClib.h>

RTC_DS3231 rtc;
int INTERRUPT_PIN = 2;
volatile int state = LOW;

void setup () {
  pinMode(INTERRUPT_PIN, INPUT_PULLUP); //internal pullup on the Arduino ensures that we can detect the LOW from the SQW/INT pin of the RTC
    
  Serial.begin(57600);
  rtc.begin();
  // rtc.adjust(DateTime(__DATE__, __TIME__)); //it is expected that the RTC is already set and running
}

void loop () {
  DateTime tt = rtc.now();
  rtc.setAlarm1Simple(tt.hour(), tt.minute() + 1);
  rtc.turnOnAlarm(1);
  if (rtc.checkAlarmEnabled(1)) {
    Serial.println("Alarm Enabled in 1 minutes");
  }
  Serial.print(tt.hour(), DEC);
  Serial.print(':');
  Serial.print(tt.minute(), DEC);
  Serial.print(':');
  Serial.println(tt.second(), DEC);
  if (rtc.checkAlarmEnabled(1) ) Serial.println("A1 Enabled"); else  Serial.println("A1 Disbled");
  Serial.println("Going to Sleep");
  Serial.flush(); // empty the send buffer, before continue with; going to sleep
  sleepNow();
  Serial.println();
  Serial.println("AWAKE");
  if( rtc.checkIfAlarm(1) ) Serial.println("Alarm1 Triggered");
}

void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(0,logData, LOW); //make sure we can be made to wake up :-) 
  sleep_mode();
  //CONTINUE HERE AFTER WAKING UP
  sleep_disable();
  detachInterrupt(0); //avoid random interrupts
  
}

void logData() {
  //do something quick, flip a flag, and handle in loop() - or do nothing at all, here
}
