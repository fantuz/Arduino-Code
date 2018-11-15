/*
  Example for receiving
  
  https://github.com/sui77/rc-switch/
  
  If you want to visualize a telegram copy the raw data and 
  paste it into http://test.sui.li/oszi/
*/

#include <RCSwitch.h>
#define button 7           //The button attached to digital pin 6
#define ledPin 13        //Onboard LED = digital pin 13

int buttonState = 1;    //Variable to control the flow of code using button presses
int buttonVal = 0;      //Variable to hold the state of the button

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT); 
  pinMode(button, INPUT);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
}

void loop() {
  buttonVal = digitalRead(button);
  if(buttonState>0 && buttonVal==HIGH){
    Serial.println("BUTTON PRESSED");
    //delay(100);
    buttonState=0;
  }
  
  buttonVal = digitalRead(button);
  if(buttonState<1 && buttonVal==HIGH){
    Serial.println("BUTTON TOGGLE");
    buttonState=1;
  }
  delay(20);

  if (mySwitch.available()) {
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(),mySwitch.getReceivedProtocol());
    mySwitch.resetAvailable();
  }
  
}
