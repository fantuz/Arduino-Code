
#include <SoftwareSerial.h>
#define B_SPEED 115200
#define S_SPEED 9600

#define MODE_PIN 9

SoftwareSerial BTSerial(10,11); // RX | TX

void setup() {
  pinMode(MODE_PIN, OUTPUT);  // this pin will pull the HC-05 pin 34 (key pin) HIGH to switch module to AT mode
  digitalWrite(MODE_PIN, HIGH);
  Serial.begin(S_SPEED);
  Serial.println("Enter AT commands:");
  BTSerial.begin(B_SPEED);
}

void loop() {
  // Keep reading from HC-05 and send to Arduino Serial Monitor
  if (BTSerial.available())
    Serial.write(BTSerial.read());

  // Keep reading from Arduino Serial Monitor and send to HC-05
  if (Serial.available())
    BTSerial.write(Serial.read());
}
