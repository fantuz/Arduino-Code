/* Flame Sensor analog example.
Code by Reichenstein7 (thejamerson.com)

Flame Sensor Ordered from DX ( http://www.dx.com/p/arduino-flame-sensor-for-temperature-
detection-blue-dc-3-3-5v-118075?tc=USD&gclid=CPX6sYCRrMACFZJr7AodewsA-Q#.U_n5jWOrjfU )

Source: www.theorycircuit.com
*/

//int Buzzer = 13; // Use buzzer for alert 
int FlamePin = 2;  // This is for input pin
int Flame = HIGH;  // HIGH when FLAME Exposed
// lowest and highest sensor readings:
const int sensorMin = 0;     // sensor minimum
const int sensorMax = 1024;  // sensor maximum

void setup() {
  // initialize serial communication @ 9600 baud:
  //Serial.begin(9600);  
//  pinMode(Buzzer, OUTPUT);
  pinMode(FlamePin, INPUT);
}

void loop() {
  // read the sensor on analog A0:
  //int sensorReading = analogRead(A0);
  //Flame = digitalRead(FlamePin);
  
  /*
  if (Flame== HIGH)
  {
    Serial.print("1,");
    //digitalWrite(Buzzer, HIGH);
  }
  else
  {
    Serial.print("0,");
    //digitalWrite(Buzzer, LOW);
  }
  Serial.println(sensorReading);
  

  /*
  // map the sensor range (four options):
  // ex: 'long int map(long int, long int, long int, long int, long int)'
  int range = map(sensorReading, sensorMin, sensorMax, 0, 3);
  
  // range value:
  switch (range) {
  case 0:    // A fire closer than 1.5 feet away.
    Serial.println("** Close Fire **");
    break;
  case 1:    // A fire between 1-3 feet away.
    Serial.println("** Distant Fire **");
    break;
  case 2:    // No fire detected.
    Serial.println("No Fire");
    break;
  }
  */
  //delay(1);  // delay between reads
  
}

