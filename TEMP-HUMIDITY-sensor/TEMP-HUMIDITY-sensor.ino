/*
 * Example code for SHT1x or SHT7x sensors demonstrating blocking calls for temperature and humidity measurement in the setup routine and
 * non-blocking calls in the main loop.  The pin 13 LED is flashed as a background task while temperature and humidity measurements are made.
 */
 
#include <Sensirion.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2 //A0

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
 
const uint8_t dataPin = 4; //22; //2;              // SHT serial data
const uint8_t sclkPin = 5; // 23; //8;              // SHT serial clock
const uint8_t ledPin  = 13;              // Arduino built-in LED
const uint32_t TRHSTEP   = 5000UL; //5000UL;       // Sensor query period
const uint32_t BLINKSTEP =  1500UL; //250      // LED blink period
 
Sensirion sht = Sensirion(dataPin, sclkPin);
 
uint16_t rawData;
float temperature;
float humidity;
float dewpoint;
 
byte ledState = 0;
byte measActive = false;
byte measType = TEMP;
 
unsigned long trhMillis = 0;             // Time interval tracking
unsigned long blinkMillis = 0;
 
void setup(void)
{
  Serial.begin(9600);
  //Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the library
  sensors.begin();
  
  pinMode(ledPin, OUTPUT);
  delay(50);                             // Wait >= 11 ms before first cmd
  // Demonstrate blocking calls
  sht.measTemp(&rawData);                // sht.meas(TEMP, &rawData, BLOCK)
  temperature = sht.calcTemp(rawData);
  sht.measHumi(&rawData);                // sht.meas(HUMI, &rawData, BLOCK)
  humidity = sht.calcHumi(rawData, temperature);
  dewpoint = sht.calcDewpoint(humidity, temperature);
  logData();
  delay(750);
}

void loop(void)
{
  unsigned long curMillis = millis();          // Get current time

  // Rapidly blink LED.  Blocking calls take too long to allow this.
  if (curMillis - blinkMillis >= BLINKSTEP) {  // Time to toggle the LED state?
    ledState ^= 1;
    digitalWrite(ledPin, ledState);
    blinkMillis = curMillis;
  }
 
  // Demonstrate non-blocking calls
  if (curMillis - trhMillis >= TRHSTEP) {      // Time for new measurements?
    measActive = true;
    measType = TEMP;
    sht.meas(TEMP, &rawData, NONBLOCK);        // Start temp measurement
    trhMillis = curMillis;
  }
  
  delay(50);
  
  if (measActive && sht.measRdy()) {           // Note: no error checking
    if (measType == TEMP) {                    // Process temp or humi?
      measType = HUMI;
      temperature = sht.calcTemp(rawData);     // Convert raw sensor data
      sht.meas(HUMI, &rawData, NONBLOCK);      // Start humidity measurement
    } else {
      measActive = false;
      humidity = sht.calcHumi(rawData, temperature); // Convert raw sensor data
      dewpoint = sht.calcDewpoint(humidity, temperature);
      logData();
    }
  }

  delay(50);
  // call sensors.requestTemperatures() to issue a global temperature request to all devices on the bus
  sensors.requestTemperatures();
  // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  Serial.println(sensors.getTempCByIndex(0));
  delay(100);
  
}
 
void logData() {
  //Serial.print("");
  Serial.print(temperature);
  Serial.print(" ");
  Serial.print(humidity);
  Serial.print(" ");
  Serial.println(dewpoint);
  //Serial.println(" C");
}

