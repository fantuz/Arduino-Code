
//#include "KTMS1201.h"
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <rotary.h>

const unsigned long max_frequency_step = 1000000; //Max Frequency step
const unsigned long max_frequency = 50000000; //Max Frequency
const int min_frequency=25; // Minimum Frequency

unsigned long last_frequency = 5000;
unsigned long frequency_step = 1;

// Rotary encoder

const int EncoderPinCLK = 21; 
const int EncoderPinDT = 20;  
const int EncoderPinSW = A7;  

/*
// KTMS1201 Pin definitions
byte CD =    9;
byte RESET = 10;
byte CS =    11;
byte N_SCK = 12;
byte SI =    13;
*/

//byte dds_RESET = 11;
//byte dds_DATA  = 10;
//byte dds_CLOCK = 12;
//byte dds_LOAD  = 13;

//Setup some items
#define W_CLK 8   // Pin 8 - connect to AD9850 module word load clock pin (CLK)
#define FQ_UD 9   // Pin 9 - connect to freq update pin (FQ)
#define DATA 10   // Pin 10 - connect to serial data load pin (DATA)
#define RESET 11  // Pin 11 - connect to reset pin (RST) 
#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW); }

Rotary r = Rotary(20,21); // sets the pins the rotary encoder uses.  Must be interrupt pins.

byte dds_RESET = 11;
byte dds_DATA  = 10;
byte dds_LOAD  = 9;
byte dds_CLOCK = 8;

int_fast32_t rx=7200000; // Starting frequency of VFO
int_fast32_t rx2=1; // variable to hold the updated frequency
int_fast32_t increment = 10; // starting VFO update increment in HZ.

int buttonstate = 0;
String hertz = "10 Hz";
int  hertzPosition = 5;
byte ones,tens,hundreds,thousands,tenthousands,hundredthousands,millions ;  //Placeholders
String freq; // string to hold the frequency
int_fast32_t timepassed = millis(); // int to hold the arduino miilis since startup
int memstatus = 1;  // value to notify if memory is current or old. 0=old, 1=current.
int ForceFreq = 1;  // Change this to 0 after you upload and run a working sketch to activate the EEPROM memory.  YOU MUST PUT THIS BACK TO 0 AND UPLOAD THE SKETCH AGAIN AFTER STARTING FREQUENCY IS SET!

LiquidCrystal lcd(28, 29, 24, 25, 26, 27);
//KTMS1201 lcd(N_SCK, SI, CD, RESET, CS);

// Updated by the ISR (Interrupt Service Routine)
unsigned volatile long frequency = 5000;

void isr ()  {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  if (interruptTime - lastInterruptTime > 5) {
    if (digitalRead(EncoderPinDT) == LOW)
    {
      frequency=frequency-frequency_step ; // Could be -5 or -10
    }
    else {
      frequency=frequency+frequency_step ; // Could be +5 or +10
    }

    frequency = min(max_frequency, max(min_frequency, frequency));

    lastInterruptTime = interruptTime;
  }
}

ISR(PCINT2_vect) {
  unsigned char result = r.process();
  if (result) {    
    if (result == DIR_CW){rx=rx+increment;}
    else {rx=rx-increment;};       
      if (rx >=30000000){rx=rx2;}; // UPPER VFO LIMIT
      if (rx <=1000000){rx=rx2;}; // LOWER VFO LIMIT
  }
}

// transfers a byte, a bit at a time, LSB first to the 9850 via serial DATA line
void tfr_byte(byte data)
{
  for (int i=0; i<8; i++, data>>=1) {
    digitalWrite(DATA, data & 0x01);
    pulseHigh(W_CLK);   //after each bit sent, CLK is pulsed high
  }
}

void setincrement(){
  if(increment == 10){increment = 50; hertz = "50 Hz"; hertzPosition=5;}
  else if (increment == 50){increment = 100;  hertz = "100 Hz"; hertzPosition=4;}
  else if (increment == 100){increment = 500; hertz="500 Hz"; hertzPosition=4;}
  else if (increment == 500){increment = 1000; hertz="1 Khz"; hertzPosition=6;}
  else if (increment == 1000){increment = 2500; hertz="2.5 Khz"; hertzPosition=4;}
  else if (increment == 2500){increment = 5000; hertz="5 Khz"; hertzPosition=6;}
  else if (increment == 5000){increment = 10000; hertz="10 Khz"; hertzPosition=5;}
  else if (increment == 10000){increment = 100000; hertz="100 Khz"; hertzPosition=4;}
  else if (increment == 100000){increment = 1000000; hertz="1 Mhz"; hertzPosition=6;}  
  else{increment = 10; hertz = "10 Hz"; hertzPosition=5;};  
   lcd.setCursor(0,1);
   lcd.print("                ");
   lcd.setCursor(hertzPosition,1); 
   lcd.print(hertz); 
   delay(250); // Adjust this delay to speed up/slow down the button menu scroll speed.
};

void storeMEM(){
  //Write each frequency section to a EPROM slot.  Yes, it's cheating but it works!
   EEPROM.write(0,millions);
   EEPROM.write(1,hundredthousands);
   EEPROM.write(2,tenthousands);
   EEPROM.write(3,thousands);
   EEPROM.write(4,hundreds);       
   EEPROM.write(5,tens);
   EEPROM.write(6,ones);   
   memstatus = 1;  // Let program know memory has been written
};

void showFreq(){
    millions = int(rx/1000000);
    hundredthousands = ((rx/100000)%10);
    tenthousands = ((rx/10000)%10);
    thousands = ((rx/1000)%10);
    hundreds = ((rx/100)%10);
    tens = ((rx/10)%10);
    ones = ((rx/1)%10);
    lcd.setCursor(0,0);
    lcd.print("                ");
   if (millions > 9){lcd.setCursor(1,0);}
   else{lcd.setCursor(2,0);}
    lcd.print(millions);
    lcd.print(".");
    lcd.print(hundredthousands);
    lcd.print(tenthousands);
    lcd.print(thousands);
    lcd.print(".");
    lcd.print(hundreds);
    lcd.print(tens);
    lcd.print(ones);
    lcd.print(" Mhz  ");
    timepassed = millis();
    memstatus = 0; // Trigger memory write
};

// frequency calc from datasheet page 8 = <sys clock> * <frequency tuning word>/2^32
void sendFrequency(double frequency) {  
  int32_t freq = frequency * 4294967295/125000000;  // note 125 MHz clock on 9850.  You can make 'slight' tuning variations here by adjusting the clock frequency.
  for (int b=0; b<4; b++, freq>>=8) {
    tfr_byte(freq & 0xFF);
  }
  tfr_byte(0x000);   // Final control byte, all 0 for 9850 chip
  pulseHigh(FQ_UD);  // Done!  Should see output
}

void show_frequency()
{
  lcd.clear();
  float display_frequency=frequency;
  String frequency_string=String(frequency);
  if (frequency<1000)
  {
    lcd.setCursor(0,0);
    lcd.print(frequency);
  }
  
  if (frequency>=1000)
  {
    lcd.setCursor(0,0);
    lcd.print(display_frequency/1000,3);
  }
}

void setup() {
  Serial.begin(9600);

  // Rotary pulses are INPUTs
  pinMode(EncoderPinCLK, INPUT);
  pinMode(EncoderPinDT, INPUT);
  
  //pinMode(A7,INPUT); // Connect to a button that goes to GND on push
  //digitalWrite(A7,HIGH);

    // Switch is floating so use the in-built PULLUP so we don't need a resistor
  pinMode(EncoderPinSW, INPUT_PULLUP);

  // Attach the routine to service the interrupts
  attachInterrupt(digitalPinToInterrupt(EncoderPinCLK), isr, LOW);
  lcd.begin(16,2);
  
  /*
  lcd.begin(16, 2);
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
  sei();
  */
  
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT); 
  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD);  // this pulse enables serial mode on the AD9850 - Datasheet page 12.
  //lcd.setCursor(hertzPosition,1);    
  //lcd.print(hertz);
  /*
  // Load the stored frequency  
  if (ForceFreq == 0) {
    freq = String(EEPROM.read(0))+String(EEPROM.read(1))+String(EEPROM.read(2))+String(EEPROM.read(3))+String(EEPROM.read(4))+String(EEPROM.read(5))+String(EEPROM.read(6));
    rx = freq.toInt();  
  }
  */ 

  delay(1000);
  //setup_dds();
  show_frequency();
  //dds(frequency);
  delay(1000);
  showFreq();
  delay(1000);
  sendFrequency(rx);
  
}

void loop() {
  // Is someone pressing the rotary switch?
  if ((!digitalRead(EncoderPinSW))) {
    while (!digitalRead(EncoderPinSW))
      delay(10);
    Serial.println("Reset");
    if (frequency_step==max_frequency_step)
    {
      frequency_step=1;
    }
    else
    {
      frequency_step=frequency_step*10;  
    }
    Serial.print("multiplier:");
    Serial.println(frequency_step);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("STEP:");
    lcd.setCursor(5,0);
    lcd.print(frequency_step);
  }
 
  if (frequency != last_frequency) {
    Serial.print(frequency > last_frequency ? "Up  :" : "Down:");
    Serial.println(frequency);
    show_frequency();
//    dds(frequency);
    last_frequency = frequency ;
  }
}

