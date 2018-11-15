
/*
  Copyright 2013-2018: (c) SAEKI Yoshiyasu, Massimiliano Fantuzzi HB3YOE
  Analog inputs attached to pins A0 through A5 (optional)
*/

#include <Bridge.h>
#include <HttpClient.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Udp.h>

#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */
#define DELAY        0
#define MAXCONN             1
#define UDP_DATAGRAM_SIZE   256
#define DNSREWRITE          256
#define HTTP_RESPONSE_SIZE  256
#define URL_SIZE            256
#define VERSION             "1.5"
#define DNS_MODE_ANSWER     1
#define DNS_MODE_ERROR      2
#define DEFAULT_LOCAL_PORT  53
#define DEFAULT_WEB_PORT    80
#define NUMT              4
#define NUM_THREADS         4
#define NUM_HANDLER_THREADS 1

#define MISO       50   //num 10 on nano
#define MOSI       51   //num 11 on nano
#define SCLK       52   //num 12 on nano
#define CLK        52
#define SS          53

//#define ETH_CS_PIN  8   //from enc28j60_tutorial-master/_18_SDWebserver/_18_SDWebserver.ino
//#define ethCSpin    8
//#define SS_PIN      53  //from MFRC522
//#define SPI_SS      53  // from RFM12B
//#define RST_LCD     53
//#define CS           8   //6
//#define SPI_SS_PIN  53  //from Robot_Control/SdCard.h
//#define SD_CS        6
//#define SPI_CS       8

//#define ETHERNET_SHIELD_SPI_CS  8;

#define PACKET_MAX_SIZE 512

#define DNS_PIN     7
#define HTTP_PIN    8

const uint8_t SD_CHIP_SELECT = 5;
const int8_t DISABLE_CHIP_SELECT = 6;

byte macOne[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEC };
byte macTwo[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//byte ip[] = {192, 168, 254, 100};
byte resIp[4] = {94,130,94,9};

unsigned int listenPort = 53;
byte remoteIp[4];
unsigned int remotePort;

char requestBuffer[PACKET_MAX_SIZE];
char responseBuffer[PACKET_MAX_SIZE];

byte serverHTTP[] = { 94, 130, 94, 9 }; // fantuz.net

boolean currentLineIsBlank = true;
char c;
//char server[] = "www.fantuz.net";    // name address for Google (using DNS)
//byte server[] = { 64, 233, 187, 99 }; // Google

const int requestInterval = 60000;  // delay between requests
boolean requested;                   // whether you've made a request since connecting
long lastAttemptTime = 0;            // last time you connected to the server, in milliseconds

IPAddress ipOne(192,168,1,241);
IPAddress ipTwo(192,168,3,241);
IPAddress dnsOne(192,168,1,1);
IPAddress dnsTwo(192,168,3,1);
IPAddress gatewayOne(192, 168,1,1);
IPAddress gatewayTwo(192, 168,3,1);
IPAddress subnetOne(255,255,255,0);
IPAddress subnetTwo(255,255,255,0);

EthernetUDP UdpTwo;

//EthernetServer server(80);
//EthernetServer serverOne(80);
//EthernetClient client;
//EthernetClient clientOne(80);
//EthernetClient clientHTTP(80); // = serverOne.available();

void checkClient(int board) {
  EthernetClient clientOne(80);
  EthernetClient client;
  
  currentLineIsBlank = true;

  if(clientOne) {
    Serial.print("client coming in...");
    Serial.println(board);
    
    while(client.connected()) {
      if(client.available()) {
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
          client.print(F("</head><body>Hello world from "));
          client.print(board);
          client.print(F("</body>"));
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

void doGET() {
  EthernetClient clientHTTP(80);
  //EthernetClient clientHTTP; // = serverOne.available();
  
  if (clientHTTP.connect(serverHTTP, 80)) {
    Serial.print(".");
    //client.println("GET /search?q=arduino HTTP/1.1");
    //clientHTTP.println("GET /asciilogo.txt HTTP/1.1");
    clientHTTP.println("GET /nslookup.php?host=google.com&type=A HTTP/1.1");
    clientHTTP.println("Host: www.fantuz.net");
    clientHTTP.println("Connection: close");
    clientHTTP.println();
  } else {
    // if you didn't get a connection to the server:
    //Serial.print("x");
  }
  lastAttemptTime = millis();


  if (clientHTTP.available()) {
    char c = clientHTTP.read();
    Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!clientHTTP.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    clientHTTP.stop();

    // do nothing forevermore:
    //while (true);
    delay(500); ///asciilogo.txt 
  }
}

void setup()
{
  Serial.begin(9600);

  /*
  if (DISABLE_CHIP_SELECT < 0) {
    //cout << F(
    //       "\nAssuming the SD is the only SPI device.\n"
    //       "Edit DISABLE_CHIP_SELECT to disable another device.\n");
  } else {
    //cout << F("\nDisabling SPI device on pin ");
    //cout << int(DISABLE_CHIP_SELECT) << endl;
    pinMode(DISABLE_CHIP_SELECT, OUTPUT);
    digitalWrite(DISABLE_CHIP_SELECT, HIGH);
  }
  */
 
  pinMode(8,OUTPUT);
  digitalWrite(8,HIGH);
  delay(50);
  pinMode(7,OUTPUT);
  digitalWrite(7,HIGH);
  delay(50);
  
  // startDNS()
  Ethernet.select(DNS_PIN);
  Ethernet.begin(macTwo, ipTwo, dnsTwo, gatewayTwo, subnetTwo);
  UdpTwo.begin(listenPort);
  //serverTwo.begin();
  delay(50);  
  //startHTTP()
  Ethernet.select(HTTP_PIN);
  Ethernet.begin(macOne, ipOne, dnsOne, gatewayOne, subnetOne);
  //EthernetServer serverOne(80);
  //serverOne.begin();
  //clientHTTP.available();
  //checkClient(1);

  delay(50);
  //checkServerGET();
  //connectToServer();

  /*
  // Bridge takes about two seconds to start up it can be helpful to use the on-board LED as an indicator for when it has initialized
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);
  */
  
  //Serial.println(F("Ready"));
  
  //SerialUSB.begin(9600);
  //while (!SerialUSB); // wait for a serial connection
}

void loop() {
  
  /*
  // Initialize the client library
  HttpClient client;

  // Make a HTTP request:
  client.get("http://www.arduino.cc/asciilogo.txt");

  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    //SerialUSB.print(c);
    Serial.print(c);
  }
  //SerialUSB.flush();

  */

  /*
  if (client.connected()) {
    if (client.available()) {
      // read incoming bytes:
      char inChar = client.read();

      // add incoming byte to end of line:
      currentLine += inChar; 

      // if you get a newline, clear the line:
      if (inChar == '\n') {
        currentLine = "";
      } 
      // if the current line ends with <text>, it will
      // be followed by the tweet:
      if ( currentLine.endsWith("<text>")) {
        // tweet is beginning. Clear the tweet string:
        readingTweet = true; 
        tweet = "";
      }
      // if you're currently reading the bytes of a tweet,
      // add them to the tweet String:
      if (readingTweet) {
        if (inChar != '<') {
          tweet += inChar;
        } 
        else {
          // if you got a "<" character,
          // you've reached the end of the tweet:
          readingTweet = false;
          Serial.println(tweet);
          
          if(tweet == ">Hello Cruel World"){
           digitalWrite(2, HIGH);
           Serial.println("LED ON!");
          }
          if(tweet != ">Hello Cruel World"){
           digitalWrite(2, LOW);
           Serial.println("LED OFF!");
          }
          
          // close the connection to the server:
          client.stop(); 
        }
      }
    }   
  }
  else if (millis() - lastAttemptTime > requestInterval) {
    // if you're not connected, and two minutes have passed since
    // your last connection, then attempt to connect again:
    connectToServer();
  }
*/  

/*
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
*/

  //int requestSize = Udp.available();
  int requestSize = UdpTwo.parsePacket();
  
  if(requestSize) {
    //Serial.print("Received packet of size ");    //Serial.println(requestSize);    //Serial.print("From ");
    IPAddress remote = UdpTwo.remoteIP();

    /*
    for (int i = 0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(UdpTwo.remotePort());
    */
    
    UdpTwo.read(requestBuffer, PACKET_MAX_SIZE);

    int type = (requestBuffer[2] >> 3) & 15;
    if(type == 0) {            // nomal request
      int ini = 12;
      int lon = requestBuffer[ini];
      char domain[64];
      int i = 0;
      while(lon != 0) {
        for(int j = 0; j < lon; j++) {
          domain[i++] = requestBuffer[ini + j + 1];
        }
        domain[i++] = '.';
        ini += lon + 1;
        lon = requestBuffer[ini];
      }
      domain[i] = '\0';
      
      //Serial.println(domain);
      
      /*
      - ANSWER
        //response[0] = 0x81;
        response[0] = 0x85;
        response[1] = 0x80;
        // Questions 1
        response[0] = 0x00;
        response[1] = 0x01;
        //Answers 1
        response[0] = 0x00;
        response[1] = 0x01;
        
      - ERROR
        response[0] = 0x81;
        response[1] = 0x82;
        // Questions 1
        response[0] = 0x00;
        response[1] = 0x01;
        // Answers 0
        response[0] = 0x00;
        response[1] = 0x00;
      */
      
      if(domain[0] != '\0') {  // request exists
        int resIndex = 0;
        
        for(int k = 0; k < 2; k++) {            // identification
          responseBuffer[resIndex++] = requestBuffer[k];
        }
        
        responseBuffer[resIndex++] = '\x85';    // response   //81
                                                // recursion desired
        responseBuffer[resIndex++] = '\x80';    // recursive
                                                // no error
        
        //for(int k = 4; k < 6; k++) {            // question
          //responseBuffer[resIndex++] = requestBuffer[k];
          responseBuffer[resIndex++] = '\x00';
          responseBuffer[resIndex++] = '\x01';
        //}
        
        for(int k = 4; k < 6; k++) {            // answer
          responseBuffer[resIndex++] = requestBuffer[k];
          //responseBuffer[resIndex++] = '\x00';
          //responseBuffer[resIndex++] = '\x01';
        }
        
        for(int k = 0; k < 4; k++) {            // authority, additional
          responseBuffer[resIndex++] = '\x00';
          //responseBuffer[resIndex++] = '\x00';
          //responseBuffer[resIndex++] = '\x00';
          //responseBuffer[resIndex++] = '\x00';
        }

        //for(int k = 12; k < requestSize -8; k++) {  // question
        for(int k = 12; k < requestSize - 11; k++) {  // question
          responseBuffer[resIndex++] = requestBuffer[k];
        }

        /*
          // Type
          response[0] = (uint8_t)(dns_req->qtype >> 8);
          response[1] = (uint8_t)dns_req->qtype;
          response+=2;
          
          // Class
          response[0] = (uint8_t)(dns_req->qclass >> 8);
          response[1] = (uint8_t)dns_req->qclass;
          response+=2;
        */
        
        responseBuffer[resIndex++] = '\xc0';    // pointer to answer
        responseBuffer[resIndex++] = '\x0c';
        
        /* TYPES */
        
  /*        
  if (dns_req->qtype == 0x0f) { //MX
          response[0] = 0x00;
          response[1] = 0x0f;
          response+=2;
  } else if (dns_req->qtype == 0xFF) { //ALL
          response[0] = 0x00;
          response[1] = 0xFF;
          response+=2;
  } else if (dns_req->qtype == 0x01) { //A
    *response++ = 0x00;
    *response++ = 0x01;
  } else if (dns_req->qtype == 0x05) { //CNAME
          response[0] = 0x00;
          response[1] = 0x05;
          response+=2;
  } else if (dns_req->qtype == 0x0c) { //PTR
          response[0] = 0x00;
          response[1] = 0x0c;
          response+=2;
  } else if (dns_req->qtype == 0x02) { //NS
          response[0] = 0x00;
          response[1] = 0x02;
          response+=2;
  } else { return; }
  */
  
        responseBuffer[resIndex++] = '\x00';    // type A
        responseBuffer[resIndex++] = '\x01';

        responseBuffer[resIndex++] = '\x00';    // class
        responseBuffer[resIndex++] = '\x01';
        
        responseBuffer[resIndex++] = '\x00';    // ttl (orig 0x3c, now 4 hours)
        responseBuffer[resIndex++] = '\x00';
        responseBuffer[resIndex++] = '\x38';    //3840
        responseBuffer[resIndex++] = '\x40';
        
        responseBuffer[resIndex++] = '\x00';    // ip length for A record
        responseBuffer[resIndex++] = '\x04';
        
        responseBuffer[resIndex++] = resIp[0];  // ip
        responseBuffer[resIndex++] = resIp[1];
        responseBuffer[resIndex++] = resIp[2];
        responseBuffer[resIndex++] = resIp[3];
        
        responseBuffer[resIndex++] = '\x00';    // end
     
        Serial.println("Contents:");
        Serial.println(requestBuffer);

        UdpTwo.beginPacket(UdpTwo.remoteIP(), UdpTwo.remotePort());
        UdpTwo.write((uint8_t *)responseBuffer, (uint16_t)(resIndex - 1));
        UdpTwo.endPacket();
        
        //Udp.sendPacket((uint8_t *)responseBuffer, (uint16_t)(resIndex - 1),remoteIp, remotePort);
        
        //return responseBuffer;
      }
    }
  }
}

