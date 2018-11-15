char mystr[32]; //Initialized variable to store received data

void setup() {
  // Begin the Serial at 9600 Baud
  Serial.begin(9600);
}

void loop() {
  // if
  while (Serial.available() > 0) {
    Serial.println();
    Serial.readBytes(mystr, 32); //Read the serial data and store in var
    Serial.print(mystr);
    Serial.println();
  }
  //Serial.println();
  
  //while (Serial.available() > 0) {Serial.read();}

  //if (Serial.available() > 0)
  //while (Serial.available() > 0)
  //  Serial.readBytes(mystr, 32);
}
