
#include <SPI.h>
#include <Ethernet.h>

byte macOne[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEC };
byte macTwo[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ipOne(192,168,1,241);
IPAddress ipTwo(192,168,3,241);
IPAddress dnsOne(192,168,1, 1);
IPAddress dnsTwo(192,168,3, 1);
IPAddress gatewayOne(192, 168, 1, 1);
IPAddress gatewayTwo(192, 168, 3, 1);
IPAddress subnet(255, 255, 255, 0);

EthernetServer serverOne(80);
EthernetServer serverTwo(80);

boolean currentLineIsBlank = true;
char c;

void setup()
{
  Serial.begin(9600);

  pinMode(8,OUTPUT);
  digitalWrite(8,HIGH);

  pinMode(7,OUTPUT);
  digitalWrite(7,HIGH);

  startOne();
  delay(500);
  startTwo();
  delay(500);
  Serial.println(F("Ready"));
}

void loop()
{
  checkServerOne();
  checkServerTwo();
}

void startOne()
{
  Serial.print(F("Starting One..."));
  Ethernet.select(8);
  Ethernet.begin(macOne, ipOne, dnsOne, gatewayOne, subnet);
  //Serial.println(Ethernet.localIP());
  serverOne.begin();
}

void startTwo()
{
  Serial.print(F("Starting Two..."));
  Ethernet.select(7);
  Ethernet.begin(macTwo, ipTwo, dnsTwo, gatewayTwo, subnet);
  //Serial.println(Ethernet.localIP());
  serverTwo.begin();
}

void checkServerOne()
{
  // select ethernet one
  Ethernet.select(8);
  EthernetClient client = serverOne.available();
  currentLineIsBlank = true;
  
  if(client)
  {
    while(client.connected())
    {
      if(client.available())
      {
        c = client.read();
        if(c == '\n' && currentLineIsBlank)
        {
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println(F("Connection: close"));
          client.println();
          client.println(F("<!DOCTYPE HTML>"));
          client.println(F("<html><head>"));
          client.println(F("<meta http-equiv=\"refresh\" content=\"5\">"));
          client.print(F("</head><body>Hello world from ethernet one!</body>"));
          client.println(F("</html>"));
          break;
        }
        if(c == '\n')
          currentLineIsBlank = true;
        else if(c != '\r')
          currentLineIsBlank = false;
      }
    }
    client.stop();
  }
}

void checkServerTwo()
{
  // select ethernet two
  Ethernet.select(7);
  EthernetClient client = serverTwo.available();
  currentLineIsBlank = true;
  
  if(client)
  {
    while(client.connected())
    {
      if(client.available())
      {
        c = client.read();
        if(c == '\n' && currentLineIsBlank)
        {
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println(F("Connection: close"));
          client.println();
          client.println(F("<!DOCTYPE HTML>"));
          client.println(F("<html><head>"));
          client.println(F("<meta http-equiv=\"refresh\" content=\"5\">"));
          client.print(F("</head><body>Hello world from ethernet two!</body>"));
          client.println(F("</html>"));
          break;
        }
        if(c == '\n')
          currentLineIsBlank = true;
        else if(c != '\r')
          currentLineIsBlank = false;
      }
    }
    client.stop();
  }
}

