
#include <LCD4Bit_mod.h>
//#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdio.h>
#define ONE_WIRE_BUS 3

char buffer[25];
byte maxlines = 2;
char line1[16];
char line2[16];
byte x = 0;
byte y = 1;
byte maxsensors = 0;

LCD4Bit_mod lcd = LCD4Bit_mod(2); 
//LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

/* DS18S20 Temperature chip i/o */
OneWire  ds(ONE_WIRE_BUS);
DallasTemperature sensors(&ds);

void flashled(void)
{
  digitalWrite(13, HIGH);
  delay(1);
  digitalWrite(13, LOW);
}

void scanSensors(void)
{
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  while (ds.search(addr))
  {
    Serial.print("R=");
    for( i = 0; i < 8; i++) {
      Serial.print(addr[i], HEX);
      Serial.print(" ");
    }
  
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.print("CRC is not valid!\n");
        return;
    }
    
    if ( addr[0] == 0x10) {
        Serial.print("Device is a DS18S20 family device.\n");
        maxsensors++;
    }
    else {
      if (addr[0] == 0x28) {
        Serial.print("Device is a DS18B20 family device.\n");
        maxsensors++;
      }
      else {
        Serial.print("Device is unknown!\n");
        Serial.print("Device ID: ");
        Serial.print(addr[0],HEX);
        Serial.println();
        return;
      }
    }
    // The DallasTemperature library can do all this work for you!
    ds.reset();
    ds.select(addr);
    ds.write(0x44,1);         // start conversion, with parasite power on at the end
    delay(1000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.
    present = ds.reset();
    ds.select(addr);    
    ds.write(0xBE);         // Read Scratchpad
    Serial.print("P=");
    Serial.print(present,HEX);
    Serial.print(" ");
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = ds.read();
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.print(" CRC=");
    Serial.print( OneWire::crc8( data, 8), HEX);
    Serial.println();
    flashled();
  }
  Serial.print("No more addresses.\n");
  ds.reset_search();
  delay(250);
}

void setup(void) {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  byte i, j;
  for (j=0;j<16;j++) 
  { 
    line1[j] = 32; 
    line2[j] = 32;
  }
  line1[16] = 0;
  line2[16] = 0;
  lcd.init();
  //lcd.init(4,1);
  lcd.clear();
  x = 0;
  y = 1;
  sensors.begin();
  scanSensors();
  lcdprintln("Temperature: ");
}

void loop(void) {
  sensors.requestTemperatures(); // minden eszköztől kérjük a hőmérséklet mérést...
  for (int i=0;i<maxsensors;i++)
  {
    flashled();
    float f = sensors.getTempCByIndex(i);  // egyesével kiolvassuk a mért értéket
    lcdprint(dtostrf(f, 5, 1, buffer));  // megjelenítjük a kijelzőn
    Serial.print("Sensor ");
    Serial.print(i,DEC);
    Serial.println(dtostrf(f, 6, 2, buffer));
  }
  // visszaugrunk a sor elejére a kijelzőn
  x = 0;
  y = 2;
}

// kijelző kezelő rutinok, hogy egyszerűen lehessen megjeleníteni szöveget
void lcdscrolllineup()
{
  y = maxlines;
  for (byte i=0;i<16;i++) { line1[i] = line2[i]; line2[i] = 32; }
  lcd.cursorTo(1,0);
  lcd.printIn(line1);
  lcd.cursorTo(2,0);
  lcd.printIn(line2);
  lcd.cursorTo(2,0);
}

void lcdprint(char msg[])
{
  byte h = strlen(msg);
  if (y>maxlines) lcdscrolllineup();
  for (byte i=0;i<h;i++) 
  {
    if (y==1) line1[x] = msg[i]; else line2[x] = msg[i];
//    lcd.cursorTo(y, x);
//    lcd.print(msg[i]);
    x++;
    if (x==16) 
    {
      x = 0;
      y++;
      if (y>maxlines) 
      {
        if (i!=h-1) lcdscrolllineup(); // van meg szoveg, scrollozni kell
      }
    }
  }
}

void lcdprintln(char msg[])
{
  lcdprint(msg);
  x = 0;
  y++;
  if (y>maxlines) lcdscrolllineup();
}

