/// This sketch implements a "as simple as possible" radio without any possibility to modify the settings after initializing the chip.\n
/// The radio chip is initialized and setup to a fixed band and frequency. These settings can be changed by modifying the 
/// FIX_BAND and FIX_STATION definitions. 
///
/// Arduino port | Si4703 signal
/// ------------ | ---------------
///     3.3V | VCC
///      GND | GND
///       A5 | SCLK
///       A4 | SDIO
///       D2 | RST
/// More documentation and source code is available at http://www.mathertel.de/Arduino

//#include <Arduino.h>
#include <Wire.h>
#include <radio.h>
#include <TEA5767.h>

#define SDA 20
#define SDIO 20
#define SCL 21
#define SCLK 21

/// The band that will be tuned by this sketch is FM.
#define FIX_BAND RADIO_BAND_FM

/// The station that will be tuned by this sketch is 89.30 MHz.
#define FIX_STATION 9440 //8930

// Create an instance of Class for Si4703 Chip
TEA5767 radio;

uint8_t test1;
byte test2;

/// Setup a FM only radio configuration with some debugging on the Serial port
void setup() {
  // open the Serial port
  Serial.begin(9600);
  Serial.println("Radio...");
  delay(500);
  // Initialize the Radio 
  radio.init();
  // Enable information to the Serial port
  radio.debugEnable();
  radio.setBandFrequency(FIX_BAND, FIX_STATION);
  radio.setVolume(2);
  radio.setMono(false);
  
}

/// show the current chip data every 3 seconds.
void loop() {
  char s[12];
  radio.formatFrequency(s, sizeof(s));
  Serial.print("Station:"); 
  Serial.println(s);
  
  Serial.print("Radio:"); 
  radio.debugRadioInfo();
  
  Serial.print("Audio:"); 
  radio.debugAudioInfo();

  delay(10000);
}

