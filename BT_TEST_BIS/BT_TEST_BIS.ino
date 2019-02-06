
#include <SoftwareSerial.h>
#define B_SPEED 115200
#define S_SPEED 9600

SoftwareSerial hc05(10,11); //2,3

void setup(){
  Serial.begin(S_SPEED);
  Serial.println("ENTER AT Commands:");
  //Initialize Bluetooth Serial Port
  hc05.begin(B_SPEED);
}

void loop(){
  //Write data from HC05 to Serial Monitor
  if (hc05.available()){
    Serial.write(hc05.read());
  }
  
  //Write from Serial Monitor to HC05
  if (Serial.available()){
    hc05.write(Serial.read());
  }

  //hc05.print("AT+BAUD8"); // Set baudrate to 115200

}
