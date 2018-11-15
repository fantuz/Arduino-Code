#include <TEA5767Radio.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal.h>

//#include <Radio>
#include "TEA576X.h"
#include "FM_Radio_Config.h"
//#include "FM_Module.h"
#include "FM_Radio.h"
//#include "TEA5767HN.h"

#define TEA576X_XTAL_32768Hz true
#define TEA5767HN_I2CBUS true

FM_Radio radio;

void setup() {
  radio.begin(LCD_BTN, MEMOMAX);
}
void loop() {
  radio.loop();
}
