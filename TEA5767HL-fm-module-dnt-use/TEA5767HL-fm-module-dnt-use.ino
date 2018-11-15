#include <TEA5767Radio.h>
#include <Wire.h>
#include <SPI.h>
#include "FM_Module.h"
#include "TEA5767HL.h"

#include "TEA576X.h"
#include "FM_Module.h"
#include "TEA5768HL.h"

FM_Module mod;
TEA5767HL tea;
//TEA576X tea;

void setup() {
  mod.begin();
  mod.searchForward();
  mod.play();
}

void loop() {
  // listening to Radio don't care here ;-)
}
