// read from eval-board

// Use the Arduino to talk to ST M24LR-discovery board. Michael Saunby. March 2013.
// http://playground.arduino.cc/code/I2CEEPROM
// http://www.developer.nokia.com/Community/Wiki/Understanding_NFC_Data_Exchange_Format_(NDEF)_messages

#include <Wire.h> //I2C library

// Deviceaddress is a 7 bit value. i.e. the read/write bit is missing and the rest is shifted right.
// i.e. deviceaddress = 0x53 is sent by Wire as 0xA6 to write or 0xA7 to read.

// Dual interface (I2C+NFC) eeprom
#define M24LR04 0x53

// I2C temperature sensor - see table 1 of data sheet.  Resistor selects address. 
#define STTS751 0x39

void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
    int rdata = data;
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(rdata);
    Wire.endTransmission();
}
  
void i2c_eeprom_write_bytes( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
    byte c;
    for ( c = 0; c < length; c++){
        Wire.beginTransmission(deviceaddress);
        Wire.write((int)(eeaddresspage >> 8)); // MSB
        Wire.write((int)(eeaddresspage & 0xFF)); // LSB
        Wire.write(data[c]);
        Wire.endTransmission();
        delay(10);
        eeaddresspage++;
    }
}

// 03 | nbytes | D1 |01 | payload_length | type | id | payload | FE
// nbytes = payload_len + 4
// payload_length = ‘enSome text here \FE’
// type = ‘T’ (text), ‘U’ (URI)
// id = 02,  03 (http://)

void i2c_eeprom_write_ndef( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
  // Write NDEF header
  char header[] = "\003\000\xD1\001\000T\002";
  header[1] = length + 5; // Total bytes after D1 marker.
  header[4] = length + 1; // Will add \xFE to payload.
  i2c_eeprom_write_bytes( deviceaddress, eeaddresspage, (byte *)header, sizeof(header) );
  i2c_eeprom_write_bytes( deviceaddress, eeaddresspage+sizeof(header)-1, data, length );
  i2c_eeprom_write_bytes( deviceaddress, eeaddresspage+sizeof(header)+length-2, (byte *)"\xFE\0\0\0", 4 );
}  

byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
  byte rdata = 0xFF;
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,1);
    if (Wire.available()){
      rdata = Wire.read();
    }
    return rdata;
  }

byte i2c_sensor_read_byte( int deviceaddress, int eeaddress ) {
  byte rdata = 0xFF;
  int rc;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)eeaddress);
  rc = Wire.endTransmission();
  Wire.requestFrom(deviceaddress,1);
  if (Wire.available()){
    rdata = Wire.read();
  }
  if(rc != 0){
    Serial.print("Error ");
    Serial.println(rc);
  } 
  return rdata;
}

void setup() {
    char message[] = "enMC SOFTWARE SAS - NFC"; // data to write. N.B. language prefix 'en'

    Wire.begin(); // initialise the connection
    Serial.begin(9600);
    i2c_eeprom_write_ndef(M24LR04, 4, (byte *)message, sizeof(message));
    Serial.println("Setup done");
  }
  
void loop() {
    int addr=13; // location of text string in NDEF payload.
    byte b, lo;
    signed char hi;
    float temperature;

    Serial.print(' ');
    while (addr<53) 
    {
      b = i2c_eeprom_read_byte(M24LR04, addr);
      if(b == 0xFE)break;
      Serial.print((char)b);
      addr++;
    }
    Serial.println(" ");
    
    // read temperature
    hi = i2c_sensor_read_byte(STTS751, 0);
    lo = i2c_sensor_read_byte(STTS751, 2);
    if( hi > 0){ 
      temperature = hi + lo * 1.0/256.0;
    }else{
      temperature = hi - lo * 1.0/256.0;
    }
    Serial.print(temperature);
    Serial.println(" ");
    delay(5000);
}
  
