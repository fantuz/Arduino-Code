#include <TEA5767Radio.h>
#include "TEA5767HN.h"
#include "TEA576X.h"

#define TEA576X_XTAL_32768Hz true
#define TEA5767HN_I2CBUS true
//     * values \c TEA576X_XTAL_32768Hz, \c TEA576X_XTAL_13MHz or \c TEA576X_XTAL_6d5MHz. 

TEA5767HN tea;
//TEA576X tea;

void setup() {
  Serial.begin(9600);
  tea.begin(TEA576X_XTAL_32768Hz, TEA5767HN_I2CBUS);
  //SEARCH_LEV_MEDIUM
  if(!tea.search(SEARCH_DIR_FORWARD, SEARCH_LEV_MEDIUM)) {
    Serial.print("Couldn't find any station!");
  }
  else {
    Serial.print("Playing station at ");
    Serial.print(tea.getFrequency()/1000000.0);
    Serial.print("MHz");
  }
}

void loop() {
  // nothing to do at present...
}

