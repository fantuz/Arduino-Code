const int triggeredLED = 7;
const int triggeredLED2 = 8;
const int RedLED = 4;        
const int GreenLED = 5;      
const int inputPin = A0;     
const int speakerPin = 3;  
const int armButton = 6;     

boolean isArmed = true;      
boolean isTriggered = false;
int buttonVal = 0;           
int prev_buttonVal = 0;     
int reading = 0;             
int threshold = 0;           

const int lowrange = 2000;   
const int highrange = 4000; 

void setup(){
  
  pinMode(triggeredLED, OUTPUT);
  pinMode(triggeredLED2, OUTPUT);
  pinMode(RedLED, OUTPUT);
  pinMode(GreenLED, OUTPUT);
  pinMode(armButton, INPUT);
  digitalWrite(triggeredLED, HIGH);
  delay(500);
  digitalWrite(triggeredLED, LOW);  

  calibrate();
  setArmedState();  
}

void loop(){
 
  reading = analogRead(inputPin);
 
  int buttonVal = digitalRead(armButton);
  if ((buttonVal == HIGH) && (prev_buttonVal == LOW)){
    setArmedState();
    delay(500);
  }

  if ((isArmed) && (reading < threshold)){
    isTriggered = true;}

  if (isTriggered){

     for (int i = lowrange; i <= highrange; i++)
    {
      tone (speakerPin, i, 250);
    }
   
    for (int i = highrange; i >= lowrange; i--)
    {
      tone (speakerPin, i, 250);
    }

    digitalWrite(triggeredLED, HIGH);
    delay(50);
    digitalWrite(triggeredLED, LOW);
    delay (50);
    digitalWrite(triggeredLED2, HIGH);
    delay (50);
    digitalWrite(triggeredLED2, LOW);
    delay (50);
    }

  delay(20);
}

void setArmedState(){

  if (isArmed){
    digitalWrite(GreenLED, HIGH);
    digitalWrite(RedLED, LOW);
    isTriggered = false;
    isArmed = false;
  } else {
    digitalWrite(GreenLED, LOW);
    digitalWrite(RedLED, HIGH);
    tone(speakerPin, 220, 125);
    delay(200);
    tone(speakerPin, 196, 250);
    isArmed = true;
  } 
}

void calibrate(){

  int sample = 0;             
  int baseline = 0;            
  const int min_diff = 200; 
  const int sensitivity = 50;
  int success_count = 0;
  
  digitalWrite(RedLED, LOW);
  digitalWrite(GreenLED, LOW);

  for (int i=0; i<10; i++){
    sample += analogRead(inputPin);
    digitalWrite(GreenLED, HIGH);
    delay (50);
    digitalWrite(GreenLED, LOW);
    delay (50); 
  }

  do
  {
    sample = analogRead(inputPin);    

    if (sample > baseline + min_diff){
      success_count++;
      threshold += sample;

      digitalWrite(GreenLED, HIGH);
      delay (100);                    
      digitalWrite(GreenLED, LOW);
      delay (100);                    
    } else {
      success_count = 0;             
      threshold = 0;
    }

  } while (success_count < 3);

  threshold = (threshold/3) - sensitivity;

  tone(speakerPin, 196, 250);
  delay(200);
  tone(speakerPin, 220, 125);
}

