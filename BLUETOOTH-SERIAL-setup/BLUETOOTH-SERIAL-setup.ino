
//#include <SoftwareSerial.h>

#define BAUDRATE 115200           

//SoftwareSerial mySerial(10, 11); // RX, TX

void setup() { 
 //Initialize serial and wait for port to open:
  Serial.begin(115200);
  //pinMode(9,OUTPUT); digitalWrite(9,HIGH);
  //Serial.println("Enter AT commands:");
  delay(10000);

  /*
  mySerial.begin(38400);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  */
  
  Serial.write("AT");
  delay(500);
  //Serial.write("AT+NAME:GENERIC");
  Serial.write("AT+NAMEGENERIC");
  delay(500);
  
  Serial.write("AT+UART=115200,0,0");
  delay(500);
  //Serial.write("AT+UART=115200");
  //delay(500);
  //Serial.write("AT+BAUD8");
  //delay(500);
  
  //  Serial.write("AT+PIN1234");
  //  delay(1000);

/*
  switch(BAUDRATE)  {
     case 1200:    Serial.write("AT+BAUD1");  break; 
     case 2400:    Serial.write("AT+BAUD2");  break; 
     case 4800:    Serial.write("AT+BAUD3");  break; 
     case 9600:    Serial.write("AT+BAUD4");  break; 
     case 19200:   Serial.write("AT+BAUD5");  break; 
     case 38400:   Serial.write("AT+BAUD6");  break;
     case 57600:   Serial.write("AT+BAUD7");  break; 
     case 115200:  Serial.write("AT+BAUD8");  break;
     default:      Serial.write("AT+BAUD4");  break; 
  }
  delay(1000);
*/  

/*
  Serial.write("AT+BAUD?");
  delay(500);
  Serial.write("AT+UART");
  delay(500);
  //Serial.write("AT+BAUD4");
  //delay(1000);
*/
  
} 


void loop() { 

/*  
  if (mySerial.available())
  Serial.write(mySerial.read());

  if (Serial.available())
  mySerial.write(Serial.read());
*/

} 

