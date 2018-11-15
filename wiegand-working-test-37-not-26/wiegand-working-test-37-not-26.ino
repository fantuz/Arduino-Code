/* Crazy People
* By Mike Cook April 2009
* Three RFID readers outputing 26 bit Wiegand code to pins:-
* Reader A (Head) Pins 2 & 3
* Interrupt service routine gathers Wiegand pulses (zero or one) until 26 have been recieved
* Then a sting is sent to processing
*/

int number[6]
int i;
//String reader1[30];
String reader1[32];
volatile int reader1Count = 0;

/*
## http://forum.arduino.cc/index.php?topic=19755.30
	Or you could use the approach I showed on the last post.
	You need to change the handling in the two interrupt service routines.
	For example in the :-

void reader1Zero(void) {
	reader1Count++;
	reader1 = reader1 << 1;
}
	declare reader1 and an array and replace:-
reader1 = reader1 << 1;
	with

for(int i=0; i<5; i++){
reader1 = (reader1 << 1);
if(reader1 > 255) {reader1 &= 0xff); reader1 [i+1] = (reader1 [i+1] << 1) | 0x1;} else reader1 [i+1] = (reader1 [i+1] << 1);
V=0;
}

	you also need to change the void reader1One(void) to shift in a 1 to the array.
	Then you need to access the number from the array not the single variable.

void ShiftIn( int V){
  for(int i=0; i<5; i++){
    number[i] = (number[i] << 1) | (V & 0x1);
    if(number[i] > 255) {number[i] &= 0xff); number[i+1] = (number[i+1] << 1) | 0x1;} else number[i+1] = (number[i+1] << 1);
    V=0;
  }
}

*/

void reader1One(void) {
  reader1[reader1Count] = "1";
  reader1Count++;
}

void reader1Zero(void) {
reader1[reader1Count] = "0";
  reader1Count++;
}


void setup()
{
  Serial.begin(9600);
  // Attach pin change interrupt service routines from the Wiegand RFID readers
  attachInterrupt(0, reader1Zero, FALLING);//DATA0 to pin 2
  attachInterrupt(1, reader1One, FALLING); //DATA1 to pin 3
  delay(10);
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
//  reader1[i] = 0;
}
  reader1Count = 0;
  digitalWrite(13, HIGH);  // show Arduino has finished initilisation
}

void loop() {
  if(reader1Count >= 31){
//  Serial.print(" Reader 1 ");Serial.println(reader1,HEX);
  Serial.println("A");
  

//for (i = 0; i < 26; i = i + 1) {
for (i = 0; i < 31; i = i + 1) {
  Serial.print(reader1[i]);
//  reader1[i] = 0;
}

Serial.println("");
  reader1Count = 0;
     }    
}
