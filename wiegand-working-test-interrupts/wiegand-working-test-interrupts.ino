/* Crazy People
* By Mike Cook April 2009
* Three RFID readers outputing 26 bit Wiegand code to pins:-
* Reader A (Head) Pins 2 & 3
* Interrupt service routine gathers Wiegand pulses (zero or one) until 26 have been recieved
* Then a sting is sent to processing
*/

volatile long reader1 = 0;
volatile int reader1Count = 0;

void reader1One(void) {
  reader1Count++;
  reader1 = reader1 << 1;
  reader1 |= 1;
}

void reader1Zero(void) {
  reader1Count++;
  reader1 = reader1 << 1;  
}

void setup() {
  Serial.begin(9600);
  // Attach pin change interrupt service routines from the Wiegand RFID readers
  attachInterrupt(0, reader1Zero, RISING);//DATA0 to pin 2
  attachInterrupt(1, reader1One, RISING); //DATA1 to pin 3
  delay(10);
  // the interrupt in the Atmel processor mixses out the first negitave pulse as the inputs are already high, so this gives 
  // a pulse to each reader input line to get the interrupts working properly. Then clear out the reader variables.
  // The readers are open collector sitting normally at a one so this is OK
  
  for(int i = 2; i<4; i++){
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH); // enable internal pull up causing a one
    digitalWrite(i, LOW); // disable internal pull up causing zero and thus an interrupt
    pinMode(i, INPUT);
    digitalWrite(i, HIGH); // enable internal pull up
  }
  
  delay(10);
  // put the reader input variables to zero
  reader1 = 0;
  reader1Count = 0;
}

void loop() {
  if(reader1Count >=26){
    //Serial.print("Reader 1 ");
    //Serial.println(reader1,HEX);
    
    //Serial.println(reader1& 0xfffffff);
    
    int serialNumber=(reader1 >> 1) & 0x3fff;
    int siteCode= (reader1 >> 17) & 0x3ff;

    Serial.print(siteCode);
    Serial.print("  ");
    Serial.println(serialNumber);
    reader1 = 0;
    reader1Count = 0;
    digitalWrite(13,HIGH);
    delay(1000);
    digitalWrite(13,LOW);
  }
}
