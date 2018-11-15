int i;
int reader1[30];
int readerTmp[30];
volatile int reader1Count = 0;

void reader1One(void) {
  reader1[reader1Count] = 1;
  reader1Count++;
}

void reader1Zero(void) {
reader1[reader1Count] = 0;
  reader1Count++;
}


void setup()
{
  Serial.begin(9600);
  // Attach pin change interrupt service routines from the Wiegand RFID readers
  attachInterrupt(0, reader1Zero, FALLING);//DATA0 to pin 2
  attachInterrupt(1, reader1One, FALLING); //DATA1 to pin 3
  delay(10);
  
  //Set pins 10/D0 and 11/D1 to output for re transmission
   pinMode(10, OUTPUT);
   pinMode(11, OUTPUT);
   
   digitalWrite(10, HIGH);
   digitalWrite(11, HIGH);
  
  
  // the interrupt in the Atmel processor mises out the first negitave pulse as the inputs are already high,
  // so this gives a pulse to each reader input line to get the interrupts working properly.
  // Then clear out the reader variables.
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
  for (i = 0; i < 30; i = i + 1) {
  reader1[i] = 0;
}
  reader1Count = 0;
  digitalWrite(13, HIGH);  // show Arduino has finished initilisation
}

void loop() {
  if(reader1Count >= 27){
    Serial.println("A");
    readerTmp[0] = reader1[25] ;
    readerTmp[1] = reader1[24] ;
    readerTmp[2] =  reader1[23] ;
    readerTmp[3] =  reader1[22] ;
    readerTmp[4] =  reader1[21] ;
    readerTmp[5] =  reader1[20] ;
    readerTmp[6] =  reader1[19] ;
    readerTmp[7] =  reader1[18] ;
    readerTmp[8] =  reader1[17] ;
    readerTmp[9] =  reader1[16] ;
    readerTmp[10] =  reader1[15] ;
    readerTmp[11] =  reader1[14] ;
    readerTmp[12] =  reader1[13] ;
    readerTmp[13] =  reader1[12] ;
    readerTmp[14] =  reader1[11] ;
    readerTmp[15] =  reader1[10] ;
    readerTmp[16] =  reader1[9] ;
    readerTmp[17] =  reader1[8] ;
    readerTmp[18] =  reader1[7] ;
    readerTmp[19] =  reader1[6] ;
    readerTmp[20] =  reader1[5] ;
    readerTmp[21] =  reader1[4] ;
    readerTmp[22] =  reader1[3] ;
    readerTmp[23] =  reader1[2] ;
    readerTmp[24] =  reader1[1] ;
    readerTmp[25] =  reader1[0] ;
  
    for (i = 0; i < 26; i = i + 1) {
    
      if(readerTmp[i] == 0){
        delay(2);
      digitalWrite(10, LOW);
      delayMicroseconds(50);
      digitalWrite(10, HIGH); 
      }else{
       delay(2);
      digitalWrite(11, LOW);
      delayMicroseconds(50);
      digitalWrite(11, HIGH); 
      }
    
      Serial.print(reader1[i]);
      reader1[i] = 0;
    }
  
    Serial.println("");
    reader1Count = 0;
  }
}

