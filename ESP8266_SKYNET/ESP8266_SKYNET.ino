/*
 *  Simple HTTP server test
 */
  
#include <ESP8266WiFi.h>

const char* ssid = "NASA";
const char* password = NULL;

WiFiServer server(80);
 
#include <Sensirion.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS A0 //2 //A0

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
//OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
//DallasTemperature sensors(&oneWire);
 
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

void setup()
{
  Serial.begin(115200);
  Serial.println();

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  IPAddress myIP = WiFi.softAPIP();
  
  Serial.println("");
  Serial.print("WiFi network name: ");
  Serial.println(ssid);
  Serial.print("WiFi network password: ");
  Serial.println(password);
  Serial.print("Host IP Address: ");
  Serial.println(myIP);
  Serial.println("");

  //sensors.begin();
  
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
  
  server.begin();
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
}

void loop()
{
  unsigned long curMillis = millis();          // Get current time

  WiFiClient client = server.available();
  // wait for a client (web browser) to connect
  if (client)
  {
    
    Serial.println("\n[Client connected]");
    while (client.connected())
    {
      // read line by line what the client (web browser) is requesting
      if (client.available())
      {
        String line = client.readStringUntil('\r');
        Serial.print(line);
        // wait for end of client's request, that is marked with an empty line
        if (line.length() == 1 && line[0] == '\n')
        {
                
          // Rapidly blink LED.  Blocking calls take too long to allow this.
          /*
            if (curMillis - blinkMillis >= BLINKSTEP) {  // Time to toggle the LED state?
            ledState ^= 1;
            digitalWrite(ledPin, ledState);
            blinkMillis = curMillis;
          }
          */
         
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
          //sensors.requestTemperatures();
          // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
          //Serial.println(sensors.getTempCByIndex(0));

          client.println(prepareHtmlPage());
          //sensors.getTempCByIndex(0);
          break;
        }
      }
    }
    delay(1000); // give the web browser time to receive the data

    // close the connection:
    client.stop();
    Serial.println("[Client disonnected]");
    delay(5000);
    logData();
  }
}

// prepare a web page to be send to a client (web browser)
String prepareHtmlPage()
{
  String htmlPage =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +  // the connection will be closed after completion of the response
            "Refresh: 5\r\n" +  // refresh the page automatically every 5 sec
            "\r\n" +
            "<!DOCTYPE HTML>" +
            "<html>" +
            //"Analog input:  " + String(analogRead(A0)) +
            //"Analog input:  " + String(sensors.getTempCByIndex(0)) +
            "Digital input:  " + String(temperature) + " C temp," + String(humidity) + " % humidity," + String(dewpoint) + " C dew" +
            "</html>" +
            "\r\n";
  return htmlPage;
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

