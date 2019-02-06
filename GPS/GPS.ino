
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

float lat = 28.5458, lon = 77.1703;

SoftwareSerial gpsSerial(4,3); //rx,tx
//LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);
TinyGPS gps;

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(4800); // connect gps sensor
  //lcd.begin(16,2);
}
 
void loop() {
  //while(gpsSerial.available()) {
    // encode gps data
    if(gps.encode(gpsSerial.read())) { 
      gps.f_get_position(&lat,&lon);
      //lcd.clear();
      //lcd.setCursor(1,0);
      //lcd.print("GPS Signal");
      //Serial.print("Position: ");
      Serial.print("Latitude:");
      Serial.print(lat,5); //6
      Serial.print(";");
      Serial.print("Longitude:");
      Serial.println(lon,5); //6
      //lcd.setCursor(1,0);
      //lcd.print("LAT:");
      //lcd.setCursor(5,0);
      //lcd.print(lat);
      //lcd.setCursor(0,1);
      //lcd.print(",LON:");
      //lcd.setCursor(5,1);
      //lcd.print(lon);
     }
  //}
  
  //String latitude = String(lat,6);
  //String longitude = String(lon,6);
  //Serial.println(latitude+";"+longitude);
  delay(1000); 
}
