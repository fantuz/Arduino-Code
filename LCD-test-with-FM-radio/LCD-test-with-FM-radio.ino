/*
AUTHOR: Enrico Formenti

The circuit:
 * LCD RS pin to digital pin 7
 * LCD Enable pin to digital pin 6
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD button value to analog pin A0
*/

#include <LiquidCrystal.h>
//LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// initialize internal variables
unsigned int l;             // current line number displayed on the first row

// text content of the menus
char *txt[4]={"line 1", "line 2", "line 3", "line 4"};

void printmenulines(unsigned int line) {
    lcd.clear();
    lcd.print(txt[line]);
    lcd.setCursor(0, 1);   
    lcd.print(txt[line+1]);     
}

void setup() {
 
  pinMode(A0, INPUT);
  lcd.begin(16, 2);
  lcd.clear();
  l = 0;
  printmenulines(0);

}

// the values below are found reading the analog pin A0
// when one of the five buttons is pressed
// no button = 1023
// B1 = 0
// B2 = 206
// B3 = 426
// B4 = 623
// B5 = 824

// the buttons are remapped to
// 5 = no button pressed
// 0 = button 1 (first button from left)
// 1 = button 2
// 2 = button 3
// 3 = button 4
// 4 = button 5

void loop() {

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
 
  uint8_t button; // code of button pressed
  boolean buttonPressed; // true if a button has been pressed
 
  button=(uint8_t)map(analogRead(0),0,1023,0,5);
 
  switch(button) {
    case 3: // down arrow
      buttonPressed = true;
      if(l<2)
        ++l;     
        break;
     case 4: // up arrow
      buttonPressed = true;
      if(l>0)
        --l;
        break;
     default:
      buttonPressed = false;
        break;
  }
 
  if(buttonPressed)
      printmenulines(l);
  delay(200);
}

