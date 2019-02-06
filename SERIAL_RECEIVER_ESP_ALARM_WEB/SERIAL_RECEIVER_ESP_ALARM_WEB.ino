
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <OneWire.h>
#include "Gsender.h"

#pragma region Globals
const char* ssid = "mps-66351";                           // WIFI network name
const char* password = "m2so-18ky-7bzz-qmjp";                       // WIFI network password
uint8_t connection_state = 0;                    // Connected to WIFI or not
uint16_t reconnect_interval = 10000;             // If not connected wait time to try again
#pragma endregion Globals

#define RELAY1    D2
#define RELAY2    D3
 
int redPin = D6;
int greenPin = D5;
int bluePin = D4;

int whitePin = 2;

int idx;

//uncomment this line if using a Common Anode LED
//#define COMMON_ANODE

const char *ssid1 = "mps-66351";
const char *password1 = "m2so-18ky-7bzz-qmjp";
//const char *ssid1 = "Ocas_Mobile";
//const char *password1 = "";

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

const int led = 2; //13

char incomingSerialData[512];
char inData[32]; // Allocate some space for the string
char inChar=-1; // Where to store the character read
byte indexxx = 0; // Index into array; where to store the character

ESP8266WebServer server(80);

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  setup_led();
  Serial.begin(9600);
  pulse_led();
  //delay(1000);
  pinMode(RELAY1, OUTPUT); delay(250);
  pinMode(RELAY2, OUTPUT); delay(250);
  pinMode(RELAY1, LOW); delay(250);
  pinMode(RELAY2, LOW); delay(250);
  
  // mix it with WIFI connection to create effects !
  digitalWrite(RELAY1,1);    delay(250);
  digitalWrite(RELAY2,1);    delay(250);
  //digitalWrite(RELAY1,1);    delay(250);
  //digitalWrite(RELAY2,1);    delay(250);

  inputString.reserve(32);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid1, password1);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);    //Serial.print(".");
  }
  
  /*
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  */
  
  if (MDNS.begin("ALARM")) {
    //Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/test.svg", drawGraph);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.on("/q1", HTTP_POST, handleQa);
  server.on("/q2", HTTP_POST, handleQb);
  server.on("/r", HTTP_GET, handleR);
  //server.on("/w", HTTP_POST, handleW);
  server.onNotFound(handleNotFound);
  server.begin();
  //Serial.println("HTTP server started");
  
  email();
}

void loop(void) {

  /*
  while (Serial.available() > 0) {
    Serial.println();
    Serial.readBytes(mystr, 32); //Read the serial data and store in var
    Serial.print(mystr);
    Serial.println();
  }
  */

  /*
  if (stringComplete) {
    Serial.print("--");
    Serial.print(inputString);
    Serial.println("--");
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  */

  while (Serial.available() > 0) {
    if(indexxx < 31) {
        inChar = Serial.read(); // Read a character
        inData[indexxx] = inChar; // Store it
        indexxx++; // Increment where to write next
        inData[indexxx] = '\0'; // Null terminate the string
        Serial.println(digitalRead(led));
    }
    indexxx = 0;
  }
  
  /*
  char mystr[512]; //Initialized variable to store received data
  char sertext[512];
  if (Serial.available() > 0) {
    Serial.readBytes(mystr, 512); //Read the serial data and store in var
    //snprintf(sertext, 32, '%s',mystr);
  }
  */

  /*
  while (Serial.available() > 0) {
    sertext = Serial.readBytes(mystr, 32); //Read the serial data and store in var
    //Serial.println();
    //Serial.readBytes(mystr, 32); //Read the serial data and store in var
    //Serial.print(mystr);
    //Serial.println();
  }
  */
  
  server.handleClient();
  
  //getCommand(incomingSerialData);
  //if (Comp("m1 on")==0) { Serial.write("Motor 1 -> Online\n"); } if (Comp("m1 off")==0) { Serial.write("Motor 1 -> Offline\n"); }

}

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}

char Comp(char* This) {
    while (Serial.available() > 0) {
        if(indexxx < 511) {
            inChar = Serial.read(); // Read a character
            inData[indexxx] = inChar; // Store it
            indexxx++; // Increment where to write next
            inData[indexxx] = '\0'; // Null terminate the string
        }
    }

    if (strcmp(inData,This)  == 0) {
        for (int i=0;i<31;i++) {
            inData[i]=0;
        }
        indexxx=0;
        return(0);
    } else {
        return(1);
    }
}

void getCommand(char *incomingSerialData) {
  int incomingSerialDataIndex = 0; // Stores the next 'empty space' in the array
  
  while(Serial.available()) {
    incomingSerialData[incomingSerialDataIndex] = Serial.read();
    incomingSerialDataIndex++;
    incomingSerialData[incomingSerialDataIndex] = '\0';
  }
}

void handleRoot() {
  digitalWrite(led,!digitalRead(led));
  //digitalWrite(led,1);
  char temp[1024];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 1024,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='10'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>fantuz.net security monitoring</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>DATA: %s</p>\
    <p>DATA: %d</p>\
    <p>LED:  %d</p>\
    <!-- <img src=\"/test.svg\" /> -->\
  </body>\
</html>",

           hr, min % 60, sec % 60, inData, inData, Serial.println(digitalRead(led)) //mystr
           );
  
  server.send(200, "text/html", temp);
  
  //digitalWrite(led, 0);
  
  //Serial3.write((uint8_t *) &temperature,4);
  //Serial3.write((uint8_t *) &humidity,4);
  //Serial3.write((uint8_t *) &dewpoint,4);

}

void handleQa() {
  /*
  if(indexxx < 31) {
      inChar = Serial.read();   // Read a character
      inData[indexxx] = inChar; // Store it
      indexxx++;                // Increment where to write next
      inData[indexxx] = '\0';   // Null terminate the string
  }
  */
  
  //indexxx = 0;
  //const uint8_t* ss = 11;
  
  Serial.write((int) 11);
  //Serial.println(char "ciao");
  //Serial.println((int) 1111000010100101);
  
  digitalWrite(led,!digitalRead(led));       // Change the state of the LED
  digitalWrite(RELAY1,!digitalRead(RELAY1));       // Change the state of the LED

  server.sendHeader("Location","/r");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                          // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleQb() {
  
  Serial.write((int) 11);
  digitalWrite(RELAY2,!digitalRead(RELAY2));       // Change the state of the LED

  server.sendHeader("Location","/r");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                          // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleR() {                          // If a POST request is made to URI /LED
  if(indexxx < 31) {
      inChar = Serial.read(); // Read a character
      inData[indexxx] = inChar; // Store it
      indexxx++; // Increment where to write next
      inData[indexxx] = '\0'; // Null terminate the string
  }
  //indexxx = 0;

  //https://projetsdiy.fr/esp8266-serveur-web-interface-graphique-html/
  //String req = client.readStringUntil('\r');
  //Serial.println(req);
  //client.flush();

  //server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(200, "text/html", "<form action=\"/q1\" method=\"POST\"><input type=\"submit\" value=\"Toggle SIREN\"></form><form action=\"/q2\" method=\"POST\"><input type=\"submit\" value=\"Toggle LIGHT\"></form>");
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/

/*
//void serialEvent1, void serialEvent2, void serialEvent3

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
*/

uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr) {
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");
    if(nSSID) {
        WiFi.begin(nSSID, nPassword);  
        Serial.println(nSSID);
    } else {
        WiFi.begin(ssid, password);
        Serial.println(ssid);
    }

    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
    {
        delay(200);
        Serial.print(".");
    }
    ++attempt;
    Serial.println("");
    if(i == 51) {
        Serial.print("Connection: TIMEOUT on attempt: ");
        Serial.println(attempt);
        if(attempt % 2 == 0)
            Serial.println("Check if access point available or SSID and Password\r\n");
        return false;
    }
    Serial.println("Connection: ESTABLISHED");
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void Awaits() {
    uint32_t ts = millis();
    while(!connection_state)
    {
        delay(50);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();
        }
    }
}

void email() {
    //connection_state = WiFiConnect();
    //if(!connection_state)  // if not connected to WIFI
    //    Awaits();          // constantly trying to connect

    Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "ESP8266 ALARM - 11, CH. CLYS, 1293, BELLEVUE";
    
    if(gsender->Subject(subject)->Send("superfantuz@gmail.com", "ESP REBOOT")) {
      for (int ii=0; ii<5; ii++) {
        setColor(255, 255, 0);  // yellow
        delay(150);
        setColor(255, 0, 0);    // red
        delay(150);
        setColor(0, 255, 0);    // green
        delay(150);
        setColor(0, 0, 255);    // blue
        delay(150);
      }
      setColor(0, 0, 0);        // transparent
      delay(100);
    } else {
      Serial.print("Error sending message: ");
      Serial.println(gsender->getError());
    }
}

void setup_led() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  //pinMode(whitePin, OUTPUT);  
}

void pulse_led() {
  setColor(255, 0, 0);    // red
  delay(100);
  setColor(0, 255, 0);    // green
  delay(100);
  setColor(0, 0, 255);    // blue
  delay(100);
  setColor(255, 255, 0);  // yellow
  delay(100);  
  setColor(80, 0, 80);    // purple
  delay(100);
  setColor(0, 255, 255);  // aqua
  delay(100);
  setColor(0, 0, 0);      // off
  delay(100);

  idx = 0;
  
  while (idx<10) {
    digitalWrite(whitePin,HIGH);
    delay(25);
    digitalWrite(whitePin,LOW);
    delay(25);
    idx++;
  }
  
  //pinMode(redPin, LOW); pinMode(greenPin, LOW); pinMode(bluePin, LOW);
}

void setColor(int red, int green, int blue)
{
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}
