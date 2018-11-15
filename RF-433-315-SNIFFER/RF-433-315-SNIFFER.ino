/*
 * RF Sniffer (C) Elia Yehuda 2014. This program was coded. No warranty whatsoever. Using this program will cause something, most likely problems.
*/
 
#include <RCSwitch.h>
#define RESEND_SNIFFED_VALUES 10
#define LED_PIN 13
 
RCSwitch rf315Switch = RCSwitch();
RCSwitch rf434Switch = RCSwitch();
 
void setup() {
  Serial.begin(9600);
  rf315Switch.enableReceive(0); // int 0, pin 2
  rf315Switch.enableTransmit(4);
  rf315Switch.setRepeatTransmit(RESEND_SNIFFED_VALUES);
  rf434Switch.enableReceive(2); // int 2, pin 21 (MEGA)
  rf434Switch.enableTransmit(5);
  rf434Switch.setRepeatTransmit(RESEND_SNIFFED_VALUES);
  Serial.println("[+] Listening");
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
 
  // flash a light to show transmission
  digitalWrite(LED_PIN, true);
  value = rfswitch.getReceivedValue();
  if (value) {
    sprintf(str, "[+] %d Received: %s / %010lu / %02d bit / Protocol = %d",
      rf, tobin32(value), value, rfswitch.getReceivedBitlength(), rfswitch.getReceivedProtocol() );
  } else {
    sprintf(str, "[-] %d Received: Unknown encoding (0)", rf);
  }
  Serial.println(str);
  //delay(2000);
  // resend the sniffed value (RESEND_SNIFFED_VALUES times)
  rfswitch.send(value, rfswitch.getReceivedBitlength());
  // reset the switch to allow more data to come
  rfswitch.resetAvailable();
  // stop light to show end of transmission
  digitalWrite(LED_PIN, false);
}
 
void loop() {
  //rf315Switch.send("000101010101111100000101");
  //delay(20);
  
  if (rf315Switch.available()) {
    process_rf_value(rf315Switch, 315);
  }
  
  //rf434Switch.send("00000000110001101100010010110010");
  //delay(10);
  if (rf434Switch.available()) {
    process_rf_value(rf434Switch, 434);
  }
}
