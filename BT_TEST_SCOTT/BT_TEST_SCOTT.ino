
#include <SoftwareSerial.h>
#define B_SPEED 115200
#define S_SPEED 9600

SoftwareSerial mySerial(10,11); // 8,9 // RX, TX
char w = ' ';
boolean NL = true;

void setup() {
  Serial.begin(S_SPEED);
  mySerial.begin(B_SPEED);
}

void loop() {
  if (mySerial.available()) {
    w = mySerial.read();
    Serial.write(w);
    mySerial.println("OK");
  }

  if (Serial.available()) {
    w = Serial.read();
    mySerial.write(w);

    if (NL) {
      Serial.print(">");
      NL = false;
    }
    Serial.write(w);
    if (w == 10) {
      NL = true;
    }
  }
}
