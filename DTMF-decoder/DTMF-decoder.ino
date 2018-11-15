
/** 
 * Basic software DTMF detection for arduino, based on https://github.com/jacobrosenthal/Goertzel code
 *  
 * Quick proof of concept, ugly code not packaged into a library :D
 * 
 * Created by Maximilien Cuony, 2016
 * Based on code by Jacob Rosenthal, June 20, 2012.
 * Released into the public domain.
 **/


#define sensorPin A0

#define NB_SAMPLES 100
#define ADCCENTER 512

const float THRESHOLD = 10000;

#if F_CPU == 16000000L
const float _SAMPLING_FREQUENCY = 8900.0;
#else
const float _SAMPLING_FREQUENCY = 4400.0;
#endif

float frequencey;
float coeff;
float Q1;
float Q2;

int samples[NB_SAMPLES];

void set_freq(float f) {

  frequencey = f;

  float omega = (2.0 * PI * frequencey) / _SAMPLING_FREQUENCY;

  coeff = 2.0 * cos(omega);

  reset_goertzel();
}
void reset_goertzel(void) {
  Q2 = 0;
  Q1 = 0;
}


void process_sample(int sample) {
  float Q0;
  Q0 = coeff * Q1 - Q2 + (float) (sample - ADCCENTER);
  Q2 = Q1;
  Q1 = Q0;
}


void sample(int sensorPin) {
  for (int index = 0; index < NB_SAMPLES; index++)   {
    samples[index] = analogRead(sensorPin);
  }
}


float detect(float f) {
  set_freq(f);
  float magnitude;

  /* Process the samples. */
  for (int index = 0; index < NB_SAMPLES; index++)   {
    process_sample(samples[index]);
  }

  /* Do the "standard Goertzel" processing. */
  magnitude = sqrt(Q1 * Q1 + Q2 * Q2 - coeff * Q1 * Q2);

  reset_goertzel();
  return magnitude;
}

void setup() {
  Serial.begin(9600);
}

char last_value;
unsigned long last_change;
char valid_value = 0;

void loop() {
  float magnitude;

  char A, B, C, D, C1, C2, C3, CA = 0;

  sample(sensorPin);

  A = detect(697) > THRESHOLD;
  B = detect(770) > THRESHOLD;
  C = detect(852) > THRESHOLD;
  D = detect(941) > THRESHOLD;


  C1 = detect(1209) > THRESHOLD;
  C2 = detect(1336) > THRESHOLD;
  C3 = detect(1477) > THRESHOLD;
  CA = detect(1633) > THRESHOLD;

  char new_value = 'X';

  if (A + B + C + D > 1) {
    new_value = '?';
  } else if (C1 + C2 + C3 + CA > 1) {
    new_value = '?';
  } else if (A && C1) {
    new_value = '1';
  } else if (A && C2) {
    new_value = '2';
  } else if (A && C3) {
    new_value = '3';
  } else if (A && CA) {
    new_value = 'A';
  } else if (B && C1) {
    new_value = '4';
  } else if (B && C2) {
    new_value = '5';
  } else if (B && C3) {
    new_value = '6';
  } else if (B && CA) {
    new_value = 'B';
  } else if (C && C1) {
    new_value = '7';
  } else if (C && C2) {
    new_value = '8';
  } else if (C && C3) {
    new_value = '9';
  } else if (C && CA) {
    new_value = 'C';
  } else if (D && C1) {
    new_value = '*';
  } else if (D && C2) {
    new_value = '0';
  } else if (D && C3) {
    new_value = '#';
  } else if (D && CA) {
    new_value = 'D';
  }

  if (last_value != new_value) {
    last_value = new_value;
    last_change = millis();
    valid_value = 0;
  } else {

    if (millis() < last_change) { // Buffer overflow each ~50days
      last_value = new_value;
      last_change = millis();
    }

    if (millis() - last_change > 100) {
      if (valid_value == 0) {
        Serial.print("Got a ");
        Serial.println(last_value);
        valid_value = 1;
      }
    }

  }
}
