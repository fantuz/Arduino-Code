/*
Adafruit Arduino - Lesson 3. RGB LED
*/
 
int redPin = 3; //11;
int greenPin = 5; //10;
int bluePin = 6; //9;
int whitePin = 13; //9;
int idx;

//uncomment this line if using a Common Anode LED
//#define COMMON_ANODE
 
void setup()
{
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(whitePin, OUTPUT);
}
 
void loop()
{
  setColor(255, 0, 0);  // red
  delay(2000);
  setColor(0, 255, 0);  // green
  delay(2000);
  setColor(0, 0, 255);  // blue
  delay(2000);
  setColor(255, 255, 0);  // yellow
  delay(2000);  
  setColor(80, 0, 80);  // purple
  delay(2000);
  setColor(0, 255, 255);  // aqua
  delay(2000);
  idx = 0;
  while (idx<10) {
    digitalWrite(13,HIGH);
    delay(100);
    digitalWrite(13,LOW);
    delay(100);
    idx++;
  }
}
 
void setColor(int red, int green, int blue)
{
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}
