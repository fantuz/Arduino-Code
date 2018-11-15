/*
SDA of TEA5767 breakout board to A4 of Arduino.
SCL of TEA5767 breakout board to A5 of Arduino.
GND of TEA5767 breakout board to GND of Arduino
VCC of TEA5767 breakout board to VCC of Arduino.

/// Original from Arduino FM receiver with TEA5767 http://www.electronicsblog.net Modified by Jingfeng Liu LinkSprite.com

*/

#include <Wire.h>
#include <LiquidCrystal.h>
#include <DFR_Key.h>

unsigned char search_mode=0;

int b=0;
int c=0;

#define Button_next 0 //6
#define Button_prev 3 //7

unsigned char frequencyH=0;
unsigned char frequencyL=0;

unsigned int frequencyB;
double frequency=0;

double freq_available=0; 

int lcd_key     = 0;
int adc_key_in  = 0;

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

//LiquidCrystal lcd(12, 11, 5, 4, 3, 2); 
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// read the buttons
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);
 // read the value from the sensor 
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE;
 // We make this the 1st option for speed reasons since it will be the most likely result

 // For V1.1 us this threshold
 /*
 if (adc_key_in < 50)   return btnRIGHT;
 if (adc_key_in < 250)  return btnUP;
 if (adc_key_in < 450)  return btnDOWN;
 if (adc_key_in < 650)  return btnLEFT;
 if (adc_key_in < 850)  return btnSELECT;
 */
 
 // For V1.0 comment the other threshold and use the one below:
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;   

 return btnNONE;  // when all others fail, return this...
}

void setup()   { 
  Wire.begin();
  lcd.begin(16, 2);
  /// buttons  
  //pinMode(Button_next, INPUT);
  //digitalWrite(Button_next, HIGH); //pull up resistor
  //pinMode(Button_prev, INPUT);
  //digitalWrite(Button_prev, HIGH); //pull up resistor

  frequency=94.4; //starting frequency
  frequencyB=4*(frequency*1000000+225000)/32768; //calculating PLL word
  frequencyH=frequencyB>>8;
  frequencyL=frequencyB&0XFF;

  delay(100);
  Wire.beginTransmission(0x60);   //writing TEA5767
  Wire.write(frequencyH);
  Wire.write(frequencyL);
  Wire.write(0xB0);
  Wire.write(0x10);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(500);
}

void loop()
{

  unsigned char buffer[5];
  lcd.setCursor(0, 0);
  Wire.requestFrom(0x60,5); //reading TEA5767

  if (Wire.available()) 
  {
    for (int i=0; i<5; i++) {
      buffer[i]= Wire.read();
    }
    freq_available=(((buffer[0]&0x3F)<<8)+buffer[1])*32768/4-225000;
    lcd.print("FM ");
    lcd.print((freq_available/1000000));
    frequencyH=((buffer[0]&0x3F));
    frequencyL=buffer[1];
    
    if (search_mode) {
      if(buffer[0]&0x80) search_mode=0;
    }
    
    if (search_mode==1) lcd.print(" SCAN");
    else {
      lcd.print("       ");
    }
    
    lcd.setCursor(0, 1);
    lcd.print("Level: ");
    lcd.print((buffer[3]>>4));
    lcd.print("/16 ");
    if (buffer[2]&0x80) lcd.print("STEREO   ");
    else lcd.print("MONO   ");
  }

  ///// buttons read
  lcd_key = read_LCD_buttons();  // read the buttons
  
  switch (lcd_key)               // depending on which button was pushed, we perform an action
     {
        case btnRIGHT:
         {
        //  if (!digitalRead(Button_next)&&!b) {
              frequency=(freq_available/1000000)+0.05;
              frequencyB=4*(frequency*1000000+225000)/32768+1;
              frequencyH=frequencyB>>8;
              frequencyL=frequencyB&0XFF;   
              Wire.beginTransmission(0x60);   
              Wire.write(frequencyH);
              Wire.write(frequencyL);
              Wire.write(0xB0);
              Wire.write(0x1F);
              Wire.write(0x00); 
              Wire.endTransmission(); 
              b=40; //100
        //  };
              delay(20);
        //  if (!digitalRead(Button_next)&&b==1) {
            ///scannnn UP
              search_mode=1;
              Wire.beginTransmission(0x60);   
              Wire.write(frequencyH+0x40);
              Wire.write(frequencyL);
              Wire.write(0xD0);
              Wire.write(0x1F);
              Wire.write(0x00); 
              Wire.endTransmission();
              b=40; //100
        //  };
              if (!b==0) b--;
              break;
         }
         
       case btnLEFT:
         {     
        //  if (!digitalRead(Button_prev)&&!c) {
              frequency=(freq_available/1000000)-0.05;
              frequencyB=4*(frequency*1000000+225000)/32768+1;
              frequencyH=frequencyB>>8;
              frequencyL=frequencyB&0XFF;
              Wire.beginTransmission(0x60);   
              Wire.write(frequencyH);
              Wire.write(frequencyL);
              Wire.write(0xB0);
              Wire.write(0x1F);
              Wire.write(0x00); 
              Wire.endTransmission(); 
              c=40; //100
        //  };
              delay(20);
        //  if (!digitalRead(Button_prev)&&c==1) {
            ///scannnn DOWN
              search_mode=1;
              Wire.beginTransmission(0x60);   
              Wire.write(frequencyH+0x40);
              Wire.write(frequencyL); 
              Wire.write(0x50);
              Wire.write(0x1F);
              Wire.write(0x00);
              Wire.endTransmission();   
              c=40; //100
        //  };          
              if (!c==0) c--;
              break;
         }
         
       case btnUP:
         {
         //lcd.print("UP    ");
         break;
         }
         
       case btnDOWN:
         {
         //lcd.print("DOWN  ");
         break;
         }
         
       case btnSELECT:
         {
         //lcd.print("SELECT");
         break;
         }
       
       case btnNONE:
         {
         //lcd.print("NONE  ");
         break;
         }
         
     }
 /*
  //////////// button_next////////// 
  if (!digitalRead(Button_next)&&!b) {
    frequency=(freq_available/1000000)+0.05;
    frequencyB=4*(frequency*1000000+225000)/32768+1;
    frequencyH=frequencyB>>8;
    frequencyL=frequencyB&0XFF;   
    Wire.beginTransmission(0x60);   
    Wire.write(frequencyH);
    Wire.write(frequencyL);
    Wire.write(0xB0);
    Wire.write(0x1F);
    Wire.write(0x00); 
    Wire.endTransmission(); 
    b=100;
  };

  if (!digitalRead(Button_next)&&b==1) {
    ///scan UP
    search_mode=1;
    Wire.beginTransmission(0x60);   
    Wire.write(frequencyH+0x40);
    Wire.write(frequencyL);
    Wire.write(0xD0);
    Wire.write(0x1F);
    Wire.write(0x00); 
    Wire.endTransmission();
    b=100;
  };    

  if (!b==0) b--;

  //////////// button_prev////////// 
  if (!digitalRead(Button_prev)&&!c) {
    frequency=(freq_available/1000000)-0.05;
    frequencyB=4*(frequency*1000000+225000)/32768+1;
    frequencyH=frequencyB>>8;
    frequencyL=frequencyB&0XFF;
    Wire.beginTransmission(0x60);   
    Wire.write(frequencyH);
    Wire.write(frequencyL);
    Wire.write(0xB0);
    Wire.write(0x1F);
    Wire.write(0x00); 
    Wire.endTransmission(); 
    c=100;
  };

  if (!digitalRead(Button_prev)&&c==1) {
    ///scan DOWN
    search_mode=1;
    Wire.beginTransmission(0x60);   
    Wire.write(frequencyH+0x40);
    Wire.write(frequencyL); 
    Wire.write(0x50);
    Wire.write(0x1F);
    Wire.write(0x00);
    Wire.endTransmission();   
    c=100;
  };          

  if (!c==0) c--;
*/
  delay(10);
}

