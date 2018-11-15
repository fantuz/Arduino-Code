/*
 * SD card attached to SPI bus as follows:
 ** CS - depends on your SD card shield or module.
 		Pin 4 used here for consistency with other Arduino examples
*/

#include <SPI.h>
#include <SD.h>

/*
//SD_CHIP_SELECT_PIN
//#define SS_PIN      53  //from MFRC522
//#define SPI_SS      53  // from RFM12B
#define SS          53
#define SPI_SS_PIN  53  //from Robot_Control/SdCard.h
//#define RST_LCD 53
#define SS 53
#define SD_CS 53
*/

//#define CS          31 //10   //8
//#define ETH_CS_PIN  31 //10   //from enc28j60_tutorial-master/_18_SDWebserver/_18_SDWebserver.ino

//#define DISABLE_CHIP_SELECT 6
//#define SDCARD_SS_PIN 5
const int chipSelect = 5; //10 //4
const int8_t DISABLE_CHIP_SELECT = 6;

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

/*
* pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
This code is required when there is a second SPI device with pin 10 as chip select on a non 328 board.
 */

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Note that even if you don't use the hardware SS pin (pin 10), it must be left as an output or the SD library won't work :
  /*
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  delay(200);
  */
  
  // SS pin must be set as Output and HIGH:
  pinMode( chipSelect, OUTPUT );
  digitalWrite( chipSelect, HIGH );
  
  Serial.print("\nInitializing SD card...");
  
  if (DISABLE_CHIP_SELECT < 0) {
  } else {
    pinMode(DISABLE_CHIP_SELECT, OUTPUT);
    digitalWrite(DISABLE_CHIP_SELECT, HIGH);
  }
  
  // we'll use the initialization code from the utility libraries since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.print("\nCard type: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  if (volumesize < 0X800000) {
    volumesize *= 512;                        // SD card blocks are always 512 bytes
    Serial.print("Volume size (bytes): ");
    Serial.println(volumesize);
    volumesize /= 1024;
  } else {
     volumesize /= 2;
  }
  /*
  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
  Serial.println(volumesize);
  */
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Gbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);


  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}


void loop(void) {

}
