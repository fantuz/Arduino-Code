/*
Look up the piezo buzzer's resonant frequency.  This one has a peak frequency of 2600 HZ shown on the cut sheet. It actually has two peak frequencies but I just chose one. The cut sheet selects the point in the middle of the two frequencies.

Next calculate the square wave you need to make on the Arduino. 1 second / 2600 = 385us (micro seconds).

The square wave is positive for half the time and neutral for half the time or 385/2 = 192us

You may use other frequencies but this is one of the loudest frequencies based on the mfg literature.

http://www.instructables.com/id/How-to-make-an-Arduino-driven-Piezo-LOUD/

*/

#include <Wiegand.h>
//int piezoPin = 5;
WIEGAND wg;

void setup() {
 Serial.begin(9600);    //Initialise Serial communication - only required if you plan to print to the Serial monitor
 //pinMode(piezoPin, OUTPUT);
 wg.begin();
}

void loop() {

//analogWrite(piezoPin, 255);  //positive square wave
//delayMicroseconds(192);      //192uS

//analogWrite(piezoPin, 0);     //neutral square wave
//delayMicroseconds(192);      //192uS

 if(wg.available()){
    Serial.print("Wiegand HEX = ");
    Serial.print(wg.getCode(),HEX);
    Serial.print(", DECIMAL = ");
    Serial.print(wg.getCode());
    Serial.print(", Type W");
    Serial.println(wg.getWiegandType());    
   }
 //delay(20);

}

/* 
  433 MHz RF REMOTE REPLAY sketch 
     Written by ScottC 24 Jul 2014
     Arduino IDE version 1.0.5
     Website: http://arduinobasics.blogspot.com
     Receiver: XY-MK-5V      Transmitter: FS1000A/XY-FST
     Description: Use Arduino to receive and transmit RF Remote signal          
 ------------------------------------------------------------- */


 /*
 #define rfReceivePin A0     //RF Receiver data pin = Analog pin 0
 #define rfTransmitPin 4  //RF Transmitter pin = digital pin 4
 #define button 6           //The button attached to digital pin 6
 #define ledPin 13        //Onboard LED = digital pin 13
 
 const int dataSize = 1500;  //Arduino memory is limited (max=1700)
 byte storedData[dataSize];  //Create an array to store the data
 const unsigned int threshold = 70;  //signal threshold value
 //100
 int maxSignalLength = 511;   //Set the maximum length of the signal
 //255
 int dataCounter = 0;    //Variable to measure the length of the signal
 int buttonState = 1;    //Variable to control the flow of code using button presses
 int buttonVal = 0;      //Variable to hold the state of the button
 int timeDelay = 105;    //Used to slow down the signal transmission (can be from 75 - 135)
//100
*/


