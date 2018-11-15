/* TM1637_4_Digit_Display_Basics.ino

The purpose of this sketch is to provide the basic structure for using the TM1637 based 4-Digit Displays
like the Grove 4 digit display or other equivalents available through the likes of www.dx.com. 
This makes use of the TM1637Display library developed by avishorp. https://github.com/avishorp/TM1637

Pin assignments are:
CLK - D9
DIO - D8
5V or 3.3V supply to Display
GND to Display

The operation is very simple.  The sketch initialises the display and then steps through the loop incrementing the value of a
variable which is then displayed on the 4-Digit display. Essentially it is the most basic function you would want from
such a display.  If you want more sophisticated functionality then use the example that ships with the library.

HC-SR04 : test program for ultrasonic module
Author: Enrico Formenti
*/

#include <TM1637Display.h>

const int CLK = 9; //Set the CLK pin connection to the display
const int DIO = 8; //Set the DIO pin connection to the display

int NumStep = 0;  //Variable to interate

TM1637Display display(CLK, DIO);  //set up the 4-Digit Display.

const uint8_t trigPin = 4;
const uint8_t echoPin = 2;
 
void setup() {
  display.setBrightness(0x0a);  //set the diplay to maximum brightness
  // initialize serial communication:
  //Serial.begin(9600);
}
 
void loop()
{
  
  uint32_t duration; // duration of the round trip
  uint32_t cm;        // distance of the obstacle
 
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);

  // Start trigger signal
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
  // convert the time into a distance
  cm = (uint32_t)((duration<<4)+duration)/1000.0; // cm = 17 * duration/1000

/*                                
  Serial.print(cm);
  Serial.print(" centimeters");
  Serial.println();
*/ 

  display.showNumberDec(cm); //Display the Variable value;
  delay(250);

  /*
  for(NumStep = 0; NumStep < 9999; NumStep++)  //Interrate NumStep
  {
    display.showNumberDec(NumStep); //Display the Variable value;
    delay(500);  //A half second delay between steps.
  }
  */
}

