/* ===============================================================
      Project: Arduino Selfie
       Author: Scott C
      Created: 14th Sept 2014
  Arduino IDE: 1.0.5
      Website: http://arduinobasics.blogspot.com/p/arduino-basics-projects-page.html
  Description: Arduino takes selfie every 30 seconds
================================================================== */

 /* 
  Connect 5V on Arduino to VCC on Relay Module
  Connect GND on Arduino to GND on Relay Module */
 
 #define CH1 8   // Connect Digital Pin 8 on Arduino to CH1 on Relay Module
 #define CH3 7   // Connect Digital Pin 7 on Arduino to CH3 on Relay Module
 
 void setup(){
   //Setup all the Arduino Pins
   pinMode(CH1, OUTPUT);
   pinMode(CH3, OUTPUT);
   
   //Turn OFF any power to the Relay channels
   digitalWrite(CH1,LOW);
   digitalWrite(CH3,LOW);
   delay(2000); //Wait 2 seconds before starting sequence
 }
 
 void loop(){
   digitalWrite(CH1, HIGH);  //Focus camera by switching Relay 1
   delay(2000);
   digitalWrite(CH1, LOW);   //Stop focus
   delay(100);
   digitalWrite(CH3, HIGH);  //Press shutter button for 0.5 seconds
   delay(500);
   digitalWrite(CH3,LOW);    //Release shutter button
   delay(30000);             //Wait 30 seconds before next selfie
 }
    
