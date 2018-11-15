
#include <Wire.h>
#include "RTClib.h" //include Adafruit RTC library
RTC_DS3231 rtc;
                       
#define RELAY1  6      
#define RELAY2  7

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char DateAndTimeString[20]; //19 digits plus the null char

void setup () {  
  delay(500);
  pinMode(RELAY1, OUTPUT);
  delay(500);
  pinMode(RELAY2, OUTPUT);
  Serial.begin(9600); //Begin the Serial at 9600 Baud
  //Print the message if RTC is not available
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  /*
  //Setup of time if RTC lost power or time is not set
  if (rtc.lostPower()) {
    //Sets the code compilation time to RTC DS3231
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  */
  delay(1000);
}

void loop () {
  //Set now as RTC time
  DateTime now = rtc.now();
  /*
  //Print RTC time to Serial Monitor
  //Serial.print("\n");
  Serial.print(now.year(), DEC);  Serial.print('/');  Serial.print(now.month(), DEC);  Serial.print('/');  Serial.print(now.day(), DEC);  Serial.print(" (");  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(")\n");
  Serial.print(now.hour(), DEC);  Serial.print(':');  Serial.print(now.minute(), DEC);  Serial.print(':');  Serial.println(now.second(), DEC);  delay(3000);
  */
  // no daysOfTheWeek[now.dayOfTheWeek()]
  sprintf_P(DateAndTimeString, PSTR("%4d-%02d-%02d %02d:%02d:%02d"), now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

  Serial.println(DateAndTimeString);
  delay(1000);
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
  
  digitalWrite(RELAY1,0);           // Turns ON Relays 1
  //Serial.println("Light ON");
  delay(1000);
  digitalWrite(RELAY2,0);           // Turns ON Relays 2
  delay(1000);
  
  digitalWrite(RELAY1,1);          // Turns OFF Relay 1
  //Serial.println("Light OFF");
  delay(1000);
  digitalWrite(RELAY2,1);          // Turns OFF Relay 2
  delay(1000);

}
