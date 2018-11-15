
#include <Wire.h>
#include <Adafruit_SI5351.h>
#include <SPI.h>
#include <Ethernet.h>

//#include <Fat16.h>
//#include <Time.h>  
//#include <DS1307RTC.h>
//#include <SdCard.cpp>
//#include <SdCard.h>
//#include <SdInfo.h>
//#include <SquawkSD.cpp>
//#include <SquawkSD.h>
//#include <JeeLib.h>
//#include <RF12.h> //from jeelabs.org
//#include <avr/eeprom.h>
//#include <PortsLCD.h> 

#include <SD.h>
//#include <tinyFAT.h>
#include <Adafruit_GFX.h>
#include "Adafruit_HX8357.h"
#include <UTFT.h>
#include <UTFT_tinyFAT.h>

#define MUXA
#define MUXB
#define MUXC
#define MUXD
#define MUX_IN
#define BUZZ
#define Serial1

#define DEBUG   1   // set to 1 to display free RAM on web page
#define SERIAL  1   // set to 1 to show incoming requests on serial port

#define CONFIG_EEPROM_ADDR ((byte*) 0x10)
#define BUFFPIXEL 20
//LCD_CS,DC_LCD,RST_LCD
#define TFT_DC 39
#define TFT_CS 38
//#define TFT_DC 9
//#define TFT_CS 10
//#define LCD_CS 38
//#define DC_LCD 39

#define LCD_CS
#define DC_LCD
#define RST_LCD

#define MISO        50   //num 10 on nano
#define MOSI        51   //num 11 on nano
#define SCLK        52   //num 12 on nano
#define CLK         52

#define SS_PIN      53  //from MFRC522
#define SPI_SS      53  // from RFM12B
//#define RST_LCD 53
//#define SPI_SS_PIN  53  //from Robot_Control/SdCard.h
#define SD_CS       53 //53
#define SS          53 // 5/6 53
//#define SPI_SS_PIN  10  //from Robot_Control/SdCard.h

//#define CS          10   //8
//#define ETH_CS_PIN  10   //from enc28j60_tutorial-master/_18_SDWebserver/_18_SDWebserver.ino
#define CS          10   //8
#define ETH_CS_PIN  10   //from enc28j60_tutorial-master/_18_SDWebserver/_18_SDWebserver.ino
#define ETHERNET_SHIELD_SPI_CS 10;

//const uint8_t SD_CHIP_SELECT = 5;
const int8_t DISABLE_CHIP_SELECT = -1;

#define REQUEST_RATE 50 // milliseconds

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

extern uint8_t BigFont[];
extern uint8_t SmallFont[];
extern uint8_t SevenSegNumFont[];

String DisplayAddress(IPAddress address) {
  return String(address[0]) + "." + 
    String(address[1]) + "." + 
    String(address[2]) + "." + 
    String(address[3]);
}

//d8:cb:8a:5b:35:14 192.168.1.93 trinity
byte mac[] = { 0xD0, 0xCB, 0x8A, 0x5B, 0x32, 0x56};

EthernetClient client(80);
String MyIpAddress;

static long timer;
//ILI9481 HX8357D CTE32HR
//UTFT tft(ITDB32S,38,39,40,41);
//UTFT myGLCD(TFT01_24_16,38,39,40,41);
UTFT myGLCD(CTE32HR,38,39,40,41);
UTFT_tinyFAT myFiles(&myGLCD);
const uint8_t chipSelect = 53; //10
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC);

Adafruit_SI5351 clockgen = Adafruit_SI5351();

char* files480[]={"JUMPERS.BMP", "aaa.bmp", "", "", "", "", "", "", "", ""}; // 480x272

int picsize_x, picsize_y;
boolean display_rendertime=false;  // Set this to true if you want the rendertime to be displayed after a picture is loaded
boolean display_filename=true;  // Set this to false to disable showing of filename

word res;
long sm, em;

// configuration, as stored in EEPROM
struct Config {
    byte band;
    byte group;
    byte collect;
    word refresh;
    byte valid; // keep this as last byte
} config;

//static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x34 };
static byte myip[] = { 192,168,1,241 };
static byte gwip[] = { 192,168,1,1 };
static byte dnsip[] = { 192,168,1,1 };
static byte pingip[] = { 192,168,1,3};

IPAddress ip(192, 168, 1, 241);
IPAddress myDns(192,168,1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

EthernetServer server(23);
boolean alreadyConnected = false; // whether or not the client was connected previously

//buffer for an outgoing data packet
//static byte outBuf[RF12_MAXDATA], outDest;
//static char outCount = -1;
//static BufferFiller bfill;  // used as cursor while filling the buffer
#define NUM_MESSAGES  10    // Number of messages saved in history
#define MESSAGE_TRUNC 15    // Truncate message payload to reduce memory use

static byte history_rcvd[NUM_MESSAGES][MESSAGE_TRUNC+1]; //history record
static byte history_len[NUM_MESSAGES]; // # of RF12 messages+header in history
static byte next_msg;       // pointer to next rf12rcvd line
static word msgs_rcvd;      // total number of lines received modulo 10,000

//byte Ethernet::buffer[1000];   // tcp/ip send and receive buffer
//const int chipSelectPin = 8;   //from MFRC522, SPI, ADDICORE, SD
const int csPin = 10; //8

const char okHeader[] PROGMEM = 
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
;

void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

void bmpDraw(char *filename, int x, int y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();
  uint8_t  lcdidx = 0;
  boolean  first = true;

  if((x >= myGLCD.getDisplayXSize()) || (y >= myGLCD.getDisplayXSize())) return;

  Serial.println();
  Serial.print("Loading image ");
  Serial.println(filename);
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.println("File not found");
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print("File size: "); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: "); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print("Bit Depth: "); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print("Image size: ");
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >=  myGLCD.getDisplayXSize())  w = myGLCD.getDisplayXSize()  - x;
        if((y+h-1) >= myGLCD.getDisplayYSize()) h = myGLCD.getDisplayYSize() - y;

        // Set TFT address window to clipped image bounds
        // tft.setAddrWindow(x, y, x+w-1, y+h-1);  not needed for RA8875
  
        for (row=0; row<h; row++) { // For each scanline...
          // Seek to start of scan line.  It might seem labor-intensive to be doing this on every line, but this method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes place if the file position actually needs to change (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each column...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              // Push LCD buffer to the display first
              if(lcdidx > 0) {
                  b = sdbuffer[buffidx++];
                  g = sdbuffer[buffidx++];
                  r = sdbuffer[buffidx++];
                  myGLCD.setColor(r,g,b);
                  myGLCD.drawPixel(x + col, y + row);
                lcdidx = 0;
                first  = false;
              }
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format
                  b = sdbuffer[buffidx++];
                  g = sdbuffer[buffidx++];
                  r = sdbuffer[buffidx++];
                  myGLCD.setColor(r,g,b);
                  myGLCD.drawPixel(x + col, y + row);
          } // end pixel
        } // end scanline
        // Write any remaining data to LCD
        if(lcdidx > 0) {
//          tft.pushColors(lcdbuffer, lcdidx, first);    replaced with drawPixel
                 myGLCD.setColor(r,g,b);
                 myGLCD.drawPixel(col, row);
            }
        Serial.print("Loaded in ");
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println("BMP format not recognized.");
}

void setup(){

  Serial.begin(9600);
  if (clockgen.begin() != ERROR_NONE) {
    Serial.print("Ooops, no Si5351 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  /* INTEGER ONLY MODE --> most accurate output */  
  /* Setup PLLA to integer only mode @ 900MHz (must be 600..900MHz) */
  /* 25MHz * m and m (the integer multipler) can range from 15 to 90! */
  /* Set Multisynth 0 to 112.5MHz using integer only mode (div by 4/6/8) */
  /* 25MHz * 36 = 900 MHz, then 900 MHz / 8 = 112.5 MHz */
  
  //Serial.println("Set PLLA to 900MHz"); //clockgen.setupPLLInt(SI5351_PLL_A, 36);
  Serial.println("Set PLLA to 600MHz");
  clockgen.setupPLLInt(SI5351_PLL_A, 16);
  
  //Serial.println("Set Output #0 to 112.5MHz");  //clockgen.setupMultisynthInt(0, SI5351_PLL_A, SI5351_MULTISYNTH_DIV_8);
  Serial.println("Set Output #0 to 75MHz");  
  //clockgen.setupMultisynthInt(0, SI5351_PLL_A, SI5351_MULTISYNTH_DIV_8);
  clockgen.setupMultisynth(0, SI5351_PLL_A, 40, 0, 1);

  /* FRACTIONAL MODE --> More flexible but introduce clock jitter */
  /* Setup PLLB to fractional mode @616.66667MHz (XTAL * 24 + 2/3) */
  /* Setup Multisynth 1 to 13.55311MHz (PLLB/45.5) */
  clockgen.setupPLL(SI5351_PLL_B, 24, 2, 3);
  Serial.println("Set Output #1 to 13.553115MHz");  
  clockgen.setupMultisynth(1, SI5351_PLL_B, 45, 1, 2);
  
  /*
    clockgen.setupMultisynth(output, SI5351_PLL_x, div, n, d);
    For the PLL input, use either SI5351_PLL_A or SI5351_PLL_B
    The final frequency is equal to the PLL / (div + n/d)
    div can range from 4 to 900
    n can range from 0 to 1,048,575
    d can range from 1 to 1,048,575
    
    Additional R Divider
    If you need to divide even more, to get to the < 100 KHz frequencies, there's an additional R divider, that divides the output once more by a fixed number:
    clockgen.setupRdiv(output, SI5351_R_DIV_x);
    output is the clock output #
    
    The R divider can be any of the following:
    SI5351_R_DIV_1
    SI5351_R_DIV_2
    SI5351_R_DIV_4
    SI5351_R_DIV_8
    SI5351_R_DIV_16
    SI5351_R_DIV_32
    SI5351_R_DIV_64
    SI5351_R_DIV_128
  */

  /* Multisynth 2 is not yet used and won't be enabled, but can be */
  /* Use PLLB @ 616.66667MHz, then divide by 900 -> 685.185 KHz */
  /* then divide by 64 for 10.706 KHz */
  /* configured using either PLL in either integer or fractional mode */

  Serial.println("Set Output #2 to 10.706 KHz");  
  clockgen.setupMultisynth(2, SI5351_PLL_B, 900, 0, 1);
  clockgen.setupRdiv(2, SI5351_R_DIV_64);
    
//  printIPAddress();
//  if (Ethernet.begin(mac) == 0) {
  Serial.println("Failed to configure Ethernet using DHCP");
  Ethernet.begin(mac, ip, myDns, gateway, subnet);

  server.begin();
  Serial.print("Chat server address:");
  Serial.println(Ethernet.localIP());
  //printIPAddress();
  myGLCD.InitLCD();
  //myGLCD.fillScr(VGA_BLACK);
  //tft.begin(CTE32HR);
  
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
  } else {
    file.initFAT(SPISPEED_VERYHIGH);
  }
  
  Serial.print(myGLCD.getDisplayXSize());
  Serial.print('x');
  Serial.println(myGLCD.getDisplayYSize());
  
//    loadConfig();
  
//  rf12_spiInit();
//  byte freq = config.band == 4 ? RF12_433MHZ :
//              config.band == 8 ? RF12_868MHZ :
//                                 RF12_915MHZ;
//

//    static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x34 };
//    static byte myip[] = { 192,168,2,51 };
//    static byte gwip[] = { 192,168,2,1 };
//    static byte dnsip[] = { 192,168,2,1 };
//    static byte pingip[] = { 192,168,2,209 };
//                                   
  //rf12_initialize(31, freq, config.group); // we never send
  //rf12_recvDone();

//  Serial.println("MAC: reading buffer ...");
//  Serial.println(sizeof Ethernet::buffer);  
    
//    if (!ether.begin(sizeof Ethernet::buffer, mymac, 8) == 0) {

    //digitalWrite(CS,HIGH);
    //pinMode(SPI_SS, OUTPUT);
    //ether.begin(sizeof Ethernet::buffer, mymac, 8);
    //ether.begin(1000, mymac, CS);
 
    /*
    if (ether.begin(sizeof Ethernet::buffer, mymac,8) == 0) 
      Serial.println( "Failed to access Ethernet controller");
    if (!ether.dhcpSetup())
      Serial.println("DHCP failed");
    Serial.println("MAC: STATIC SETUP ...");

    if (!ether.begin(sizeof Ethernet::buffer, mymac, 8) == 0) {
//      lcd.setCursor(0, 1);
//      lcd.print("....49 FAIL ....");
    }
    */ 
    //else {      lcd.print(" .... whatever");    }

//    ether.staticSetup(myip, gwip, dnsip);
//    //ether.staticSetup(myip);
////    ether.printIp("IP: ", ether.myip);
//    
//    Serial.println("IP -> set as follows");
//    ether.printIp("  IP:  ", ether.myip);
//    ether.printIp("  GW:  ", ether.gwip);  
//    ether.printIp("  DNS: ", ether.dnsip);
//      
//    //ether.parseIp(ether.hisip, pingip);
//    //ether.printIp("SRV: ", pingip);
//    ether.registerPingCallback(gotPinged);
//
//    Serial.println("PING READY");
////    ether.persistTcpConnection(true); 
////    lcd.setCursor(0, 0);
////    lcd.print("    ETH OK      ");
//
//    Serial.println("MAC: STATIC SETUP COMPLETED ...");
//    //Serial.flush();
//    //pinMode(8, OUTPUT);
//    //digitalWrite(8,LOW);
//    //digitalWrite(CS,HIGH);
//    //pinMode(SPI_SS, OUTPUT);

//  ip = Ethernet.localIP();
//  for (byte thisByte = 0; thisByte < 4; thisByte++) {
//    // print the value of each byte of the IP address:
//    Serial.print(ip[thisByte], DEC);
//    Serial.print(".");
//  }
//  char *ip_addr[31];
//  memset(ip_addr,0,31);
//  sprintf(ip_addr,"IP: %d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
//  //myGLCD.print(ip_addr, CENTER, 35);

  myGLCD.clrScr();
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.setColor(0, 255, 0);
  //myGLCD.setFont(SmallFont); //Font

  ip = Ethernet.localIP();
  char *c = " ";
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(ip[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println(); 
  
  itoa(Serial.read(),c,10);
  //sprintf(ip,"IP: %d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
  myGLCD.print(c, CENTER, 35);
  
  //* thisChar
  MyIpAddress = String(Ethernet.localIP());
  char buf[16]; // 3*"nnn." + 1*"nnn" + NUL
  snprintf(buf, sizeof(buf),"%d %d %d %d", ip[0], ip[1], ip[2], ip[3]);
  myGLCD.print(buf, 0, 0);
  delay(500);
  
  myGLCD.clrScr();
  //bmpDraw("JUMPERS.BMP", 0, 0);
  //delay(2000);
  //myGLCD.clrScr();

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

  /* Enable the clocks */
  clockgen.enableOutputs(true);
  //clockgen.enableOutputs(0);
}

void loop() {
   
  // wait for a new client:
  EthernetClient client = server.available();
  int written = 0;
/*
  while(!client){
    ; // wait until there is a client connected to proceed
  }
*/
  // when the client sends the first byte, say hello:
  while (client) {
    
    //server.write(thisChar);
    
    /*
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
      Serial.print(Ethernet.localIP()[thisByte], DEC);
      Serial.print(".");
    }
    */
    
    if (written = 0) {
      char bufxx[16]; // 3*"nnn." + 1*"nnn" + NUL
      snprintf(bufxx, sizeof(bufxx)," %d\.%d\.%d\.%d ",Ethernet.localIP()[0],Ethernet.localIP()[1],Ethernet.localIP()[2],Ethernet.localIP()[3]);
      server.write(bufxx);
      written = 1;
    }
    
    /*
    if (!alreadyConnected) {
      // clear out the input buffer:
      client.flush();
      Serial.println("We have a new client");
      alreadyConnected = true;
    }
    */
    if (client.available() > 0) {
    //for (byte cnt = 0; cnt < 8; cnt++) {
      char thisChar = client.read();
      server.write(thisChar);
        
      if((48 < thisChar < 57) || (65 < thisChar < 90) || (97 < thisChar < 122)) {
        String myString = String(thisChar);
        myGLCD.setFont(BigFont);
        myGLCD.print(myString, 0, 0);
        delay(50);
        myGLCD.clrScr();
      }
    }
  }
  
//  myGLCD.fillScr(VGA_BLACK);
//  myGLCD.setColor(VGA_WHITE);
//  myGLCD.setBackColor(VGA_BLACK);
  
//  myGLCD.print("TEST PAUSE", 0, 0);
//  ip = Ethernet.localIP();
//  char *c = " ";
//  for (byte thisByte = 0; thisByte < 4; thisByte++) {
//    // print the value of each byte of the IP address:
//    Serial.print(ip[thisByte], DEC);
//    Serial.print("."); 
//  }
//     
//  itoa(Serial.read(),c,10);
//  myGLCD.print(c, CENTER, 35);
//  myGLCD.print(itoa(Ethernet.localIP(),c,10),CENTER, 35);

  //var = printIPAddress();
  //char buffer[20];
  //float temp=printIPAddress();
  //dtostrf(temp, buffer, 19,1);
  //buffer = printIPAddress();
  //myGLCD.print(char,0,0);

//  bmpDraw("aaa.bmp", 170, 136);
    //myGLCD.setFont(SevenSegNumFont); //BigFont
    //myGLCD.print("Text rotation", 180, 0);

//  Serial.println(Ethernet.localIP());        //displays: 2253433024
//  Serial.println(MyIpAddress);               //displays: i.e. 2253433024 (octal)

//  myFiles.loadBitmap(0, 0, 170, 136, "aaa.bmp");
//  myFiles.loadBitmap(0, 0, 170, 136, "JUMPERS.BMP");
  
    myGLCD.setColor(0, 0, 255);
    //myGLCD.setFont(SmallFont);
    //myGLCD.setFont(SevenSegNumFont);
    myGLCD.setFont(BigFont);
    
    /*
    myGLCD.print("0 degrees", 0, 16, 0);
    myGLCD.print("90 degrees", 319, 0, 90);
    myGLCD.print("180 degrees", 319, 239, 180);
    myGLCD.print("270 degrees", 0, 239, 270);

    myGLCD.print("0 degrees", 0, 16, 0);
    myGLCD.print("90 degrees", 479, 0, 90);
    myGLCD.print("180 degrees", 479, 319, 180);
    myGLCD.print("270 degrees", 0, 319, 270);
 
    myGLCD.setColor(0, 255, 0);
    myGLCD.print("45", 159, 159, 45);
    myGLCD.print("90", 239, 100, 90);
    myGLCD.print("180", 319, 220, 180);
    */
    
    /*
    myGLCD.print(ip, 79, 0);
    timer = micros();
    delay(500);
    */

    //word len = ether.packetReceive();
    //word pos = ether.packetLoop(len);
    byte pingip[] = { 192,168,1,3 };
    
//  Serial.println(sizeof Ethernet::buffer);  
//  Serial.println(sizeof len);  
  
// report whenever a reply to our outgoing ping comes back
//  if (len > 0 && ether.packetLoopIcmpCheckReply(ether.hisip)) {
    ////rf12_recvDone();
    
//    if (len > 0 && ether.packetLoopIcmpCheckReply(pingip)) {
//      Serial.print("good pinging response");
//    Serial.print((micros() - timer) * 0.001, 3);
//    Serial.println(" ms");
//    }

/*
  // check if valid tcp data is received
  if (pos) {
      bfill = ether.tcpOffset();
      char* data = (char *) Ethernet::buffer + pos;
      Serial.println("VALID PACKET num ");
      Serial.println(data);

      // receive buf hasn't been clobbered by reply yet
      if (strncmp("GET / ", data, 6) == 0)
          homePage(bfill);
      else if (strncmp("GET /c", data, 6) == 0)
          configPage(data, bfill);
      else if (strncmp("GET /s", data, 6) == 0)
          sendPage(data, bfill);
      else
          bfill.emit_p(PSTR(
              "HTTP/1.0 401 Unauthorized\r\n"
              "Content-Type: text/html\r\n"
              "\r\n"
              "<h1>401 Unauthorized</h1>"));  
      ether.httpServerReply(bfill.position()); // send web page data
  }
*/

// RFM12 loop runner, don't report acks
//    if (rf12_recvDone() && rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0) {
//        history_rcvd[next_msg][0] = rf12_hdr;
//        for (byte i = 0; i < rf12_len; ++i)
//            if (i < MESSAGE_TRUNC) 
//                history_rcvd[next_msg][i+1] = rf12_data[i];
//        history_len[next_msg] = rf12_len < MESSAGE_TRUNC ? rf12_len+1
//                                                         : MESSAGE_TRUNC+1;
//        next_msg = (next_msg + 1) % NUM_MESSAGES;
//        msgs_rcvd = (msgs_rcvd + 1) % 10000;
//
//        if (RF12_WANTS_ACK && !config.collect) {
//#if SERIAL
//            Serial.println(" -> ack");
//#endif
//            //rf12_sendStart(RF12_ACK_REPLY, 0, 0, 1);
//            //rf12_sendStart(RF12_ACK_REPLY, 0, 0);
//        }
//    }

  ip = Ethernet.localIP();
  char *c = " ";
  /*
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(ip[thisByte], DEC);
    Serial.print("."); 
  }

  Serial.println();
  */
  
  char buf[16]; // 3*"nnn." + 1*"nnn" + NUL
  itoa(Serial.read(),c,10);
  
//  dtostrf
//  itoa
//  example -> lcd.print(Wire.read(), HEX);
//  char q[16] = sprintf(ip,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
//  char *q = sprintf(ip,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);

  snprintf(buf, sizeof(buf),"%d\.%d\.%d\.%d",ip[0], ip[1], ip[2], ip[3]);
  itoa(buf,c,10);
  myGLCD.print(buf, 200, 0);
  delay(500);

  /*
  myGLCD.printNumI(ip[0],0,0);
  myGLCD.printNumI(ip[1],100,50);
  myGLCD.printNumI(ip[2],0,150);
  myGLCD.printNumI(ip[3],150,150);
  */
  
  myGLCD.print(buf, CENTER, 35);
  delay(500);

  myGLCD.clrScr();
}

