
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

#define button  6 //The button attached to digital pin 6
#define ledPin 13

//Variable to control the flow of code using button presses
int buttonState = 1;
//Variable to hold the state of the button
int buttonVal = 0;

void blinker() {
  for (int q=0; q<40; q=q+1){
    digitalWrite(ledPin, HIGH);
    delay(20);
    digitalWrite(ledPin,LOW);
    delay(20);
  }
}

void listenForSignal() {
  if (mySwitch.available()) {
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(),mySwitch.getReceivedProtocol());
    mySwitch.resetAvailable();
  }
  delay(100);
  blinker();
}

void sendSignal() {
  mySwitch.send("000101010101111100000101");
  //blinker();
  delay(100);
}

void setup() {
  mySwitch.enableTransmit(4); // Transmitter is connected to Arduino Pin #10 // #4 
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  // Optional set protocol (default is 1, will work for most outlets)
  mySwitch.setProtocol(1);
  // Optional set pulse length.
  mySwitch.setPulseLength(216); //320
  // Optional set number of transmission repetitions.
  //mySwitch.setRepeatTransmit(2);

  Serial.begin(9600);
}

void loop() {
  buttonVal = digitalRead(button);
  
  if(buttonState>0 && buttonVal==HIGH){
    Serial.println("Listening for Signal");
    listenForSignal();
    int buttonVal = 0;
    buttonState=0;
    //delay(250);
    delay(4000);
    Serial.println("Send Signal");
    //sendSignal();
  }
  
  buttonVal = digitalRead(button);
  //if(buttonState<1 && buttonVal==HIGH){
  //}
  buttonState=1;
}

/* See Example: TypeA_WithDIPSwitches */
//mySwitch.switchOn("11111", "00010");

/* Same switch as above, but using decimal code */
//mySwitch.send(5393, 24);

/* Same switch as above, but using binary code */
//mySwitch.send("000101010101111100000101");

/* Same switch as above, but tri-state code */ 
//mySwitch.sendTriState("00000FFF0F0F");
