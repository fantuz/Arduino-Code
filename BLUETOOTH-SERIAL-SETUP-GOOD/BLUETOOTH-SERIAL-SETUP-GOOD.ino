                 
//If youre not using a BTBee connect set the pin connected to the KEY pin high
#include <SoftwareSerial.h>
SoftwareSerial BTSerial(10, 11); // RX, TX //(4,5);
 
void setup() {
  String setName = String("AT+NAMEGENERIC\r\n"); //Setting name as 'MyBTBee'
  
  Serial.begin(38400);
  BTSerial.begin(115200);

  delay(500);
  BTSerial.print("AT\r\n"); //Check Status
  delay(500);
  
  BTSerial.print("AT+UART\r\n"); //Check Status
  delay(500);
  
  while (BTSerial.available()) {
      Serial.write(BTSerial.read());
  }
    
  BTSerial.print(setName); //Send Command to change the name
  delay(500);
  
  while (BTSerial.available()) {
      Serial.write(BTSerial.read());
  }
}

void loop() {

}

/*
AT Check connection status.
AT+NAME ="ModuleName" Set a name for the device
AT+ADDR Check MAC Address
AT+UART Check Baudrate
AT+UART="9600"  Sets Baudrate to 9600
AT+PSWD Check Default Passcode
AT+PSWD="1234"  Sets Passcode to 1234
*/

