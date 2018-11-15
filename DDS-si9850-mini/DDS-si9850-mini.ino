/*
* AD9850 based frequency generator using a rotary encoder for frequency tuning.
* 
* Based on  AD9851 DDS sketch by Andrew Smallbone at www.rocketnumbernine.com
*
* Author: Tim Sinaeve 
* (12/02/2015)
*/

// Include the library code
#include <Wire.h>  // Comes with Arduino IDE
// note: the 'standard LiquidCrystal_I2C.h' does not work for us.
#include "LiquidCrystal_I2C.h"
#include <Switch.h>

// Pin definitions
const byte ENCODER_BUTTON_PIN          = 6;
const byte ENCODER_A_PIN               = 7;
const byte ENCODER_B_PIN               = 8;

const byte AD9850_WORD_LOAD_CLOCK_PIN  = 2;     // (CLK)
const byte AD9850_FREQUENCY_UPDATE_PIN = 3;     // (FQ)
const byte AD9850_SERIAL_DATA_LOAD_PIN = 4;    // (DATA)
const byte AD9850_RESET_PIN            = 5;    // (RST).

const unsigned long AD9850_CLOCK_FREQUENCY = 125000000;

// Settings
const unsigned long MIN_FREQ      = 0;
const unsigned long MAX_FREQ      = 630000000; // max 63 MHz
const unsigned long MAX_DELTA     = 100000000;
const unsigned long DEFAULT_DELTA = 10000;

unsigned long delta   = 10000;
unsigned long freq    = 0;
unsigned long reading = 0;

// Timing for polling the encoder
unsigned long lastTime;
unsigned long currentTime;

// Storing the readings
boolean encA;
boolean encB;
boolean encButton;
boolean lastA = false;

// Objects
Switch toggleSwitch = Switch(ENCODER_BUTTON_PIN);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void pulseHigh(byte pin)
{
  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}

// Sends a byte, a bit at a time, LSB first to the AD9850 via serial DATA line
void sendByte(byte data)
{
  for (int i = 0; i < 8; i++, data >>= 1)
  {
    digitalWrite(AD9850_SERIAL_DATA_LOAD_PIN, data & 0x01);
    //after each bit sent, CLK is pulsed high
    pulseHigh(AD9850_WORD_LOAD_CLOCK_PIN);
  }
}

// frequency calc from datasheet page 8 = <sys clock> * <frequency tuning word>/2^32
void sendFrequency(double frequency)
{
  int32_t f = frequency * 4294967295 / AD9850_CLOCK_FREQUENCY;
  for (int b = 0; b < 4; b++, f >>= 8)
  {
    sendByte(f & 0xFF);
  }
  sendByte(0x000); // final control byte, all 0 for 9850 chip
  pulseHigh(AD9850_FREQUENCY_UPDATE_PIN);
}

boolean readEncoderValue()
{
  boolean b = false;
  if (toggleSwitch.longPress())
  {
    reading = 10000;
    delta = 1000;
    b = true;
  }
  if (toggleSwitch.doubleClick())
  {
    reading = MIN_FREQ;
    delta = 10000;
    b = true;
  }
  if (toggleSwitch.poll())
  {
    if (toggleSwitch.on())
    {
      delta *= 10;
      if (delta == MAX_DELTA)
        delta = 1;
      b = true;
    }
  }

  // read elapsed time
  currentTime = millis();
  if (currentTime >= (lastTime + 5))
  {
    // read encoder movement
    encA = digitalRead(ENCODER_A_PIN);
    encB = digitalRead(ENCODER_B_PIN);
    // check if A has gone from high to low
    if ((!encA) && (lastA))
    {
      // check if B is high
      if (encB)
      {
        // clockwise
        if (reading + delta <= MAX_FREQ)
        {
          reading = reading + delta;
        }
      }
      else
      {
        // anti-clockwise
        if (reading - delta >= MIN_FREQ)
        {
          reading = reading - delta;
        }
        else
        {
          reading = 0;
        }
      }
    }
    // store reading of A and millis for next loop
    lastA = encA;
    lastTime = currentTime;
  }
  b = b || (freq != reading);
  return b;
}

void setup()
{
  // configure arduino data pins for output
  pinMode(AD9850_FREQUENCY_UPDATE_PIN, OUTPUT);
  pinMode(AD9850_WORD_LOAD_CLOCK_PIN, OUTPUT);
  pinMode(AD9850_SERIAL_DATA_LOAD_PIN, OUTPUT);
  pinMode(AD9850_RESET_PIN, OUTPUT);

  // set the two pins as inputs with internal pullups
  pinMode(ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN, INPUT_PULLUP);

  // setup AD9850 module
  pulseHigh(AD9850_RESET_PIN);
  pulseHigh(AD9850_WORD_LOAD_CLOCK_PIN);
  pulseHigh(AD9850_FREQUENCY_UPDATE_PIN);  // this pulse enables serial mode - Datasheet page 12 figure 10

  lcd.begin(16, 2);
  lcd.backlight();

  // Set up the timing of the polling
  currentTime = millis();
  lastTime  = currentTime;
}

void loop()
{
  if (readEncoderValue())
  {
    freq = reading;
    char s[16];
    sprintf(s, "%10ld Hz", reading);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(s);
    int p = int(log10(delta));
    lcd.setCursor(9 - p, 1);
    lcd.print("^");
    sendFrequency(reading);
  }
}
