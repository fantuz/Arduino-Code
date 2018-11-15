
#include <SPI.h>
#include <SD.h>
/*
//#define RST_LCD 53
#define SS 53
#define SD_CS 53

//SD_CHIP_SELECT_PIN
//#define SS_PIN      53  //from MFRC522
//#define SPI_SS      53  // from RFM12B
#define SS          53
#define SPI_SS_PIN  53  //from Robot_Control/SdCard.h
#define SDCARD_SS_PIN 53

#define CS          10   //8
#define ETH_CS_PIN  10   //from enc28j60_tutorial-master/_18_SDWebserver/_18_SDWebserver.ino
*/
const int chipSelect = 6; //10 //4

File myFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time, so you have to close this one before opening another.
  myFile = SD.open("test-1.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test-1.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test-1.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test-1.txt");
  if (myFile) {
    Serial.println("test-1.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test-1.txt");
  }
}

void loop() {
  // nothing happens after setup
}

