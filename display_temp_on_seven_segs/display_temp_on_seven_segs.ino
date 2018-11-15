#include <Arduino.h>
#include <TM1637Display.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/pgmspace.h>

//circuit gnd-10k=(a0)=(white)...sensor....black-5v
#define THERM_PIN   0  // 10ktherm & 10k resistor as divider.
#define ONE_WIRE_BUS 2 /*-(Connect to Pin 2 )-*/
#define DIO 3
#define CLK 4

#define TEST_DELAY   200

TM1637Display display(CLK, DIO);
OneWire ourWire(ONE_WIRE_BUS);
DallasTemperature sensors(&ourWire);

//a I _ up
//f I left up
//b I right up

//d _ low
//e I left low
//c I right low

//g - mid

const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,           // E
	};

const uint8_t SEG_0000[] = {
   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
};

const uint8_t SEG_0[] = {
   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
};

const uint8_t SEG_1[] = {
   SEG_B | SEG_C,   
};

const uint8_t SEG_2[] = {
   SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
};  

const uint8_t SEG_3[] = {
   SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,
};  

const uint8_t SEG_4[] = {
   SEG_B | SEG_C | SEG_F | SEG_G,
};  

const uint8_t SEG_5[] = {
   SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
};  

const uint8_t SEG_6[] = {
   SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
};  

const uint8_t SEG_7[] = {
   SEG_A | SEG_B | SEG_C,
};  

const uint8_t SEG_8[] = {
   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
};  

const uint8_t SEG_9[] = {
   SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,
};  


int led_state;
unsigned long time;

//Thermistor lookup Table
//
// Small Version
/*
ADC   C
250,  1.4
275,  4.0
300,  6.4
325,  8.8
350,  11.1
375,  13.4
400,  15.6
425,  17.8
450,  20.0
475,  22.2
500,  24.4
525,  26.7
550,  29.0
575,  31.3
600,  33.7
625,  36.1
650,  38.7
675,  41.3
700,  44.1
725,  47.1
750,  50.2
775,  53.7
784,  55.0
825,  61.5
850,  66.2
875,  71.5
900,  77.9
925,  85.7
937,  90.3
950,  96.0
975,  111.2
1000, 139.5
*/

// Big lookup Table (approx 750 entries), subtract 238 from ADC reading to start at 0*C. Entries in 10ths of degree i.e. 242 = 24.2*C Covers 0*C to 150*C For 10k resistor/10k thermistor voltage divider w/ therm on the + side.
const int temps[] PROGMEM = { 0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 143, 144, 145, 146, 147, 148, 149, 150, 151, 151, 152, 153, 154, 155, 156, 157, 158, 159, 159, 160, 161, 162, 163, 164, 165, 166, 167, 167, 168, 169, 170, 171, 172, 173, 174, 175, 175, 176, 177, 178, 179, 180, 181, 182, 182, 183, 184, 185, 186, 187, 188, 189, 190, 190, 191, 192, 193, 194, 195, 196, 197, 197, 198, 199, 200, 201, 202, 203, 204, 205, 205, 206, 207, 208, 209, 210, 211, 212, 212, 213, 214, 215, 216, 217, 218, 219, 220, 220, 221, 222, 223, 224, 225, 226, 227, 228, 228, 229, 230, 231, 232, 233, 234, 235, 235, 236, 237, 238, 239, 240, 241, 242, 243, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 252, 253, 254, 255, 256, 257, 258, 259, 260, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 422, 423, 424, 425, 426, 427, 428, 429, 430, 432, 433, 434, 435, 436, 437, 438, 439, 441, 442, 443, 444, 445, 446, 448, 449, 450, 451, 452, 453, 455, 456, 457, 458, 459, 460, 462, 463, 464, 465, 466, 468, 469, 470, 471, 472, 474, 475, 476, 477, 479, 480, 481, 482, 484, 485, 486, 487, 489, 490, 491, 492, 494, 495, 496, 498, 499, 500, 501, 503, 504, 505, 507, 508, 509, 511, 512, 513, 515, 516, 517, 519, 520, 521, 523, 524, 525, 527, 528, 530, 531, 532, 534, 535, 537, 538, 539, 541, 542, 544, 545, 547, 548, 550, 551, 552, 554, 555, 557, 558, 560, 561, 563, 564, 566, 567, 569, 570, 572, 574, 575, 577, 578, 580, 581, 583, 585, 586, 588, 589, 591, 593, 594, 596, 598, 599, 601, 603, 604, 606, 608, 609, 611, 613, 614, 616, 618, 620, 621, 623, 625, 627, 628, 630, 632, 634, 636, 638, 639, 641, 643, 645, 647, 649, 651, 653, 654, 656, 658, 660, 662, 664, 666, 668, 670, 672, 674, 676, 678, 680, 683, 685, 687, 689, 691, 693, 695, 697, 700, 702, 704, 706, 708, 711, 713, 715, 718, 720, 722, 725, 727, 729, 732, 734, 737, 739, 741, 744, 746, 749, 752, 754, 757, 759, 762, 764, 767, 770, 773, 775, 778, 781, 784, 786, 789, 792, 795, 798, 801, 804, 807, 810, 813, 816, 819, 822, 825, 829, 832, 835, 838, 842, 845, 848, 852, 855, 859, 862, 866, 869, 873, 877, 881, 884, 888, 892, 896, 900, 904, 908, 912, 916, 920, 925, 929, 933, 938, 942, 947, 952, 956, 961, 966, 971, 976, 981, 986, 991, 997, 1002, 1007, 1013, 1019, 1024, 1030, 1036, 1042, 1049, 1055, 1061, 1068, 1075, 1082, 1088, 1096, 1103, 1110, 1118, 1126, 1134, 1142, 1150, 1159, 1168, 1177, 1186, 1196, 1206, 1216, 1226, 1237, 1248, 1260, 1272, 1284, 1297, 1310, 1324, 1338, 1353, 1369, 1385, 1402, 1420, 1439, 1459, 1480, 1502 };

void printDouble( double val, byte precision){
   // prints val with number of decimal places determine by precision
   // precision is a number from 0 to 6 indicating the desired decimial places
   // example: printDouble( 3.1415, 2); // prints 3.14 (two decimal places)
   val=val/10;
   Serial.print(int(val));  //prints the int part
   if( precision > 0) {
     Serial.print("."); // print the decimal point
     unsigned long frac;
     unsigned long mult = 1;
     byte padding = precision -1;
     while(precision--)
        mult *=10;
       
     if(val >= 0)
       frac = (val - int(val)) * mult;
     else
       frac = (int(val)- val ) * mult;
     unsigned long frac1 = frac;
     while( frac1 /= 10 )
       padding--;
     while(  padding--)
       Serial.print("0");
     Serial.println(frac,DEC) ;
   }
}

void setup() {
    
  Serial.begin(9600);
  Serial.println("Temperature Sensor 1: DS18B20");
  Serial.println("Temperature Sensor 2: one-wire");
  delay(1000);

  sensors.begin();
  pinMode(13, OUTPUT);
  time=millis();

}

void loop() { 

  int therm;
  //double therm2;  
  
  int k;
  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  display.setBrightness(0x0f);
      
  Serial.print("\nRequesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  delay(50);
  therm=analogRead(THERM_PIN)-238;
  therm=pgm_read_word(&temps[therm]);
  //therm2=therm;
  Serial.println("DONE");
  
  //Serial.println(therm2);   //Serial.println(therm, 2, DEC);   //Serial.println(therm, DEC);
  Serial.print("Device 1 (index 0) = ");   printDouble(therm, 2); //   Serial.print(" Degrees C\n");
  Serial.print("Device 2 (index 0) = ");   Serial.println(sensors.getTempCByIndex(0)); //  Serial.println(" Degrees C");
//  Serial.print("Device 1 (index 0) = "); //  Serial.print(sensors.getTempFByIndex(0)); //  Serial.println(" Degrees F");

  if (millis() - time >2000){   
    time=millis();
    digitalWrite(13,led_state);
    led_state=!led_state;
  }

  // All segments on
  display.setSegments(data);
  display.setSegments(SEG_0000);
  delay(TEST_DELAY);
  
  // Selectively set different digits
//  data[0] = 0b01001001;
//  data[1] = display.encodeDigit(1);
//  data[2] = display.encodeDigit(2);
//  data[3] = display.encodeDigit(3);

//   
//  // Show decimal numbers with/without leading zeros
//  bool lz = false;
//  for (uint8_t z = 0; z < 2; z++) {
//	for(k = 0; k < 10000; k += k*4 + 7) {
//		display.showNumberDec(k, lz);
//		delay(TEST_DELAY);
//	}
//	lz = true;
//  }
  
  // Show decimal number whose length is smaller than 4
//  for(k = 0; k < 4; k++)
//	data[k] = 0;
//  display.setSegments(data);
  
  //display.showNumberDec(153, false, 3, 1);
  //delay(TEST_DELAY);
  //display.showNumberDec(22, false, 2, 2);
  //delay(TEST_DELAY);
  //display.showNumberDec(0, true, 1, 3);
  //delay(TEST_DELAY);
  //display.showNumberDec(0, true, 1, 2);
  //delay(TEST_DELAY);
  //display.showNumberDec(0, true, 1, 1);
  //delay(TEST_DELAY);
  //display.showNumberDec(0, true, 1, 0);
  //delay(TEST_DELAY);

  // Brightness Test
  for(k = 0; k < 8; k++)
//	data[k] = 0x00;
	data[k] = 0xff;

  for(k = 0; k < 32; k++) {
    display.setBrightness(k);
    display.setSegments(data);
    delay(TEST_DELAY);
  }
    
  // Done!
  display.setSegments(SEG_0);
  delay(TEST_DELAY);
  display.setSegments(SEG_1);
  delay(TEST_DELAY);
  display.setSegments(SEG_2);
  delay(TEST_DELAY);
  display.setSegments(SEG_3);
  delay(TEST_DELAY);
  display.setSegments(SEG_4);
  delay(TEST_DELAY);
  display.setSegments(SEG_5);
  delay(TEST_DELAY);
  display.setSegments(SEG_6);
  delay(TEST_DELAY);
  display.setSegments(SEG_7);
  delay(TEST_DELAY);
  display.setSegments(SEG_8);
  delay(TEST_DELAY);
  display.setSegments(SEG_9);
  delay(TEST_DELAY);
  display.setSegments(SEG_DONE);
  delay(TEST_DELAY);
  //while(1);


}
