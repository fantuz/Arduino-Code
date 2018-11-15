/*
  Example for different sending methods
  
  https://github.com/sui77/rc-switch/
  
*/

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {

  Serial.begin(9600);
  
  // Transmitter is connected to Arduino Pin #10  // #4
  mySwitch.enableTransmit(4);
  
  // Optional set protocol (default is 1, will work for most outlets)
  // mySwitch.setProtocol(2);

  // Optional set pulse length.
  mySwitch.setPulseLength(360);
  
  // Optional set number of transmission repetitions.
  mySwitch.setRepeatTransmit(5);
  
}

void loop() {

  /* See Example: TypeA_WithDIPSwitches */
  //mySwitch.switchOn("11111", "00010");
  //delay(1000);
  //mySwitch.switchOff("11111", "00010");
  //delay(1000);

  /* Same switch as above, but using decimal code */
  //mySwitch.send(5393, 24);
  //delay(1000);  
  //mySwitch.send(5396, 24);
  //delay(1000);  

  // max's room
  // on 001110100100011110110001
  mySwitch.send("001110100100011110110010");
  delay(1000);
  
  //ground floor
  // on 000111100011011000110011
  mySwitch.send("000111100011011000110010");
  delay(1000);
  
  //1st floor
  // on 110001101100010010110001
  mySwitch.send("110001101100010010110010");
  delay(1000);

  // 2 rcv
  // 101001000001000000001000 // pulse 200

  // 1 rcv
  // 111001101010100000001000 // pulse 200

  // button
  // 000101010101111100000101 // tristate 0FFFFF1100FF // pulse 220

  // black & white rcv
  // 000101010101111100000000 // tristate 0FFFFF110000 // pulse 380

  /* Same switch as above, but tri-state code */ 
  //mySwitch.sendTriState("00000FFF0F0F");
  //delay(1000);  
  //mySwitch.sendTriState("00000FFF0FF0");
  //delay(1000);

  //delay(20000);
}
