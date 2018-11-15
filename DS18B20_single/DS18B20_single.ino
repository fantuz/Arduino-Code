/* YourDuino TEST: Temperature Sensor DS18B20
 *  V1.2 08-26-2016
- Connect cable to Arduino Digital I/O Pin 2
terry@yourduino.com */

/*-----( Import needed libraries )-----*/
#include <OneWire.h>
#include <DallasTemperature.h>

/*-----( Declare Constants )-----*/
#define ONE_WIRE_BUS 2 /*-(Connect to Pin 2 )-*/

/*-----( Declare objects )-----*/
/* Set up a oneWire instance to communicate with any OneWire device*/
OneWire ourWire(ONE_WIRE_BUS);

/* Tell Dallas Temperature Library to use oneWire Library */
DallasTemperature sensors(&ourWire);

/*-----( Declare Variables )-----*/


void setup() /*----( SETUP: RUNS ONCE )----*/
{
/*-(start serial port to see results )-*/
delay(1000);
Serial.begin(9600);
Serial.println("YourDuino.com: Temperature Sensor Test Program");
Serial.println("Temperature Sensor: DS18B20");
delay(1000);

/*-( Start up the DallasTemperature library )-*/
sensors.begin();
}/*--(end setup )---*/


void loop() /*----( LOOP: RUNS CONSTANTLY )----*/
{
Serial.println();
Serial.print("Requesting temperature...");
sensors.requestTemperatures(); // Send the command to get temperatures
Serial.println("DONE");

Serial.print("Device 1 (index 0) = ");
Serial.print(sensors.getTempCByIndex(0));
Serial.println(" Degrees C");
Serial.print("Device 1 (index 0) = ");
Serial.print(sensors.getTempFByIndex(0));
Serial.println(" Degrees F");
delay(5000);
}/* --(end main loop )-- */

/* ( THE END ) */

