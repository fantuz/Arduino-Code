
#include <Ports.h>
#include <avr/sleep.h>

#include <RF12sio.h>
#include <Ports.h>
#include <RF69_compat.h>
#include <RF69.h>
#include <PortsSHT11.h>
#include <RF69_avr.h>
#include <PortsBMP085.h>
#include <JeeLib.h>
#include <RF12.h>
#include <PortsLCD.h>
//#include <avr/sleep.h>
//#include <isp/sleep.h>

//#define F_CPU 14745600L
//#define BAUD  921600 //57600
#define BAUD  57600

#include <util/setbaud.h>
#include <util/delay.h>

long counter;
int led = 7;

void setup () {
  //Serial.begin(9600);
  pinMode(led, OUTPUT);     
  Serial.begin(9600);
  Serial.println("[beacon_max]");
  // node 11, group 17, 868 MHz
  //rf12_initialize(11, RF12_868MHZ, 17);
  rf12_initialize(25, RF12_433MHZ, 4);
  counter=0;
}

void loop () {
  Serial.println("[counter]");
  ++counter;
  delay(15000);
  //Serial.print(rf12_data[0], DEC);
  Serial.println(" #");
  Serial.println(counter);
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  Serial.println("[initialized...]");

  rf12_sendNow(0, &counter, sizeof counter);
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  Serial.println(" #");
  Serial.println(counter);
  delay(15000);
}

