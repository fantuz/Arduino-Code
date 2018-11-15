
#include <SPI.h>
#include <Ethernet.h>

byte macOne[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEC };
byte macTwo[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ipOne(192,168,1,241);
IPAddress ipTwo(192,168,3,241);
IPAddress dnsOne(192,168,1,1);
IPAddress dnsTwo(192,168,3,1);
IPAddress gatewayOne(192, 168, 1, 1);
IPAddress gatewayTwo(192, 168, 3, 1);
IPAddress subnet(255, 255, 255, 0);

void setup() {
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
}

void startOne() {
  Serial.print(F("Starting One..."));
  Ethernet.select(8);
  Ethernet.begin(macOne, ipOne, dnsOne, gatewayOne, subnet);
  Serial.println(Ethernet.localIP());
}

void startTwo() {
  Serial.print(F("Starting Two..."));
  Ethernet.select(7);
  Ethernet.begin(macTwo, ipTwo, dnsTwo, gatewayTwo, subnet);
  Serial.println(Ethernet.localIP());
}

