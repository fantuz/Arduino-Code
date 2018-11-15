// many sensors supported, plus motion sensor...

#include <SPI.h>
#include <MySensor.h>  
#include <DHT.h>  
#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS 5 // Pin where dallase sensor is connected 
#define MAX_ATTACHED_DS18B20 16
#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_MOT 2   // Id of the sensor child
#define CHILD_ID_DS1 3
#define HUMIDITY_SENSOR_DIGITAL_PIN 4

#define DIGITAL_INPUT_SENSOR 3   // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
#define INTERRUPT DIGITAL_INPUT_SENSOR-2 // Usually the interrupt = pin -2 (on uno/nano anyway)

unsigned long SLEEP_TIME = 5000; // Sleep time between reads (in milliseconds)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

MySensor gw;
DHT dht;
float lastTemp;
float lastHum;
float lastTemperature[MAX_ATTACHED_DS18B20];
int numSensors=0;
boolean receivedConfig = false;
boolean metric = false; 
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgMot(CHILD_ID_MOT, V_TRIPPED);
MyMessage msgOneWire(0, V_TEMP);

void setup()  
{ 
  sensors.begin();
  gw.begin();
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN); 

  // Send the Sketch Version Information to the Gateway
  gw.sendSketchInfo("Temp/Humidity/Motion", "1.0");

  // Fetch the number of attached temperature sensors  
  numSensors = sensors.getDeviceCount();

 pinMode(DIGITAL_INPUT_SENSOR, INPUT);      // sets the motion sensor digital pin as input
 
  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_MOT, S_MOTION);
  for (int j=CHILD_ID_DS1; j < (numSensors + CHILD_ID_DS1) && j<MAX_ATTACHED_DS18B20; j++) {   
  gw.present(j, S_TEMP);
  }
   
  metric = gw.getConfig().isMetric;
}

void loop()      

{  
  // Process incoming messages (like config from server)
  gw.process();
  
  // Fetch temperatures from Dallas sensors
  sensors.requestTemperatures();
  
  // Read digital motion value
  boolean tripped = digitalRead(DIGITAL_INPUT_SENSOR) == HIGH; 
  Serial.print("Motion: ");      
  Serial.println(tripped);
  gw.send(msgMot.set(tripped?"1":"0"));  // Send tripped value to gw 
  
  delay(dht.getMinimumSamplingPeriod());

  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT");
  } else if (temperature != lastTemp) {
    lastTemp = temperature;
    if (!metric){
    temperature = dht.toFahrenheit(temperature);
    }
    gw.send(msgTemp.set(temperature, 1));
    Serial.print("DHT Temp: ");
    Serial.println(temperature);
  }
  
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
      Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum) {
      lastHum = humidity;
      gw.send(msgHum.set(humidity, 1));
      Serial.print("DHT RH: ");
      Serial.println(humidity);
  }

  // Read temperatures and send them to controller 
  for (int i=0; i < numSensors && i<MAX_ATTACHED_DS18B20; i++) {
//int i = 0;
    // Fetch and round temperature to one decimal
    float temp = static_cast<float>(static_cast<int>((sensors.getTempCByIndex(i)) * 10.)) / 10.;
 
    // Only send data if temperature has changed and no error
    if (lastTemperature[i] != temp && temp != -127.00) {
 
      // Send in the new temperature
      Serial.print("DS");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(temp);
      gw.send(msgOneWire.setSensor(i+CHILD_ID_DS1).set(temp, 1));
      lastTemperature[i]=temp;
    }
  }
 
  // Sleep until interrupt comes in on motion sensor. Send update every two minutes. 
  gw.sleep(INTERRUPT,CHANGE, SLEEP_TIME);
}
