/* YourDuino Multiple DS18B20 Temperature Sensors on 1 wire
  DS18B20 Pinout (Left to Right, pins down, flat side toward you)
  - Left   = Ground   - Center = Signal (Pin 2):  (with 3.3K to 4.7K resistor to +5 or 3.3 )   - Right  = +5 or +3.3 V
   Questions: terry@yourduino.com     V1.01  01/17/2013 ...based on examples from Rik Kretzinger
// Get 1-wire Library here: http://www.pjrc.com/teensy/td_libs_OneWire.html //Get DallasTemperature Library here:  http://milesburton.com/Main_Page?title=Dallas_Temperature_Control_Library ... or not ??
*/
#include <OneWire.h>
#include </home/max/Downloads/AR18/arduino-root/libraries/DallasTemperature/DallasTemperature.h>                          // older or newer ? same README, same PROPERTIES....
//#include </home/max/Downloads/AR18/arduino-root/libraries/DS18B20/DallasTemperature.h>                                    // both the exact same
//#include </home/max/Downloads/AR18/arduino-root/libraries/Arduino-Temperature-Control-Library/DallasTemperature.h>      // both the exact same

/*
  /home/max/Downloads/AR18/arduino-root/libraries/DallasTemperature
  /home/max/Downloads/AR18/arduino-root/libraries/DS18B20 
  /home/max/Downloads/AR18/arduino-root/libraries/Arduino-Temperature-Control-Library
*/
 
#define ONE_WIRE_BUS_PIN 2
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS_PIN);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Assign the addresses of your 1-Wire temp sensors.
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html

DeviceAddress Probe01 = {0x28, 0x9F, 0x27, 0xB8, 0x04, 0x00, 0x00, 0x31 };
DeviceAddress Probe02 = {0x29, 0x91, 0x63, 0x13, 0x00, 0x00, 0x00, 0x4A };
//DeviceAddress Probe01 = { 0x28, 0x8A, 0xB1, 0x40, 0x04, 0x00, 0x00, 0xC7 }; 
//DeviceAddress Probe02 = { 0x28, 0xCC, 0x92, 0x40, 0x04, 0x00, 0x00, 0xB6 };
//DeviceAddress Probe03 = { 0x28, 0x4D, 0x8D, 0x40, 0x04, 0x00, 0x00, 0x78 };
//DeviceAddress Probe04 = { 0x28, 0x9A, 0x80, 0x40, 0x04, 0x00, 0x00, 0xD5 };
//DeviceAddress Probe05 = { 0x28, 0xE1, 0xC7, 0x40, 0x04, 0x00, 0x00, 0x0D };
//uint8_t aaaa = sensors.requestTemperaturesByAddress(0x29, 0x91, 0x63, 0x13, 0x00, 0x00, 0x00, 0x4A);

void setup()   /****** SETUP: RUNS ONCE ******/
{
  // start serial port to show results
  Serial.begin(9600);
  //Serial.print("Initializing Temperature Control Library Version ");
  
  // Initialize the Temperature measurement library
  sensors.begin();
  //Serial.println(DALLASTEMPLIBVERSION);

  // set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
  sensors.setResolution(Probe01, 10);
  sensors.setResolution(Probe02, 12);
  //sensors.setResolution(Probe03, 10);
  //sensors.setResolution(Probe04, 10);
  //sensors.setResolution(Probe05, 10);

}//--(end setup )---

void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  delay(1000);
  Serial.println();
  Serial.print("Number of Devices found on bus = ");  
  Serial.println(sensors.getDeviceCount());   
  Serial.print("Getting temperatures... ");  
  Serial.println();   
  
  // Command all devices on bus to read temperature  
  sensors.requestTemperatures();  
  
  Serial.print("Probe 01 temperature is:   ");
  printTemperature(Probe01);
  Serial.println();
  //printwhatYouHave(Probe01);
  //Serial.println();
  //printVoltage(Probe01);
  //Serial.println();

  Serial.print("Probe 02 temperature is:   ");
  printTemperature(Probe02);
  Serial.println();
  //printwhatYouHave(Probe02);
  //Serial.println();
  //printVoltage(Probe02);
  //Serial.println();
 
  //char humidRAW = sensors.readScratchPad(DeviceAddress,12);
  //float aaaa sensors.(Probe2);
  //sensors.requestTemperatures();
  delay(3000);
  //Serial.print(DallasTemperature::readPowerSupply(voltageV));
  //Serial.print(voltageV);
  //   Serial.print(aaaa);
  //printVoltage();
  
  delay(3000);

  //Serial.print("Probe 03 temperature is:   ");
  //printTemperature(Probe03);
  //Serial.println();
   
  //Serial.print("Probe 04 temperature is:   ");
  //printTemperature(Probe04);
  //Serial.println();
  
  //Serial.print("Probe 05 temperature is:   ");
  //printTemperature(Probe05);
  //Serial.println();
   
  
}//--(end main loop )---

//void printVoltage()
//{
   //const uint8_t voltageV = sensors.readPowerSupply(deviceAddress);
   //uint8_t voltageV = sensors.readPowerSupply(deviceAddress);
   //const unsigned char* aaa = sensors.readPowerSupply(deviceAddress);
   //bool aaaa = sensors.readPowerSupply(deviceAddress);
   //float aaaa = sensors.requestTemperaturesByAddress(deviceAddress);
   //const uint8_t aaaa = sensors.requestTemperaturesByAddress(deviceAddress);
   //float aaaa = sensors.requestTemperatures(deviceAddress);
   //Serial.print(DallasTemperature::requestTemperaturesByAddress(deviceAddress));
//   Serial.print(DallasTemperature::DallasTemperature::requestTemperaturesByIndex(deviceAddress));
//}

void printTemperature(DeviceAddress deviceAddress)
{

float tempC = sensors.getTempC(deviceAddress);


   if (tempC == -127.00) 
   {
      Serial.print("Error getting temperature  ");
   } 
   else
   {
      Serial.print("C: ");
      Serial.print(tempC);
      //Serial.print(" F: ");
      //Serial.print(DallasTemperature::toFahrenheit(tempC));
   }
   delay(5000);
}// End printTemperature
//*********( THE END )***********

