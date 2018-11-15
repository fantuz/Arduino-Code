
#include <Wire.h>
#include <Adafruit_SI5351.h>

Adafruit_SI5351 clockgen = Adafruit_SI5351();

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Si5351 Clockgen Test"); Serial.println("");
  
  if (clockgen.begin() != ERROR_NONE) {
    Serial.print("Ooops, no Si5351 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  Serial.println("OK!");

  /* INTEGER ONLY MODE --> most accurate output */  
  /* Setup PLLA to integer only mode @ 900MHz (must be 600..900MHz) */
  /* 25MHz * m and m (the integer multipler) can range from 15 to 90! */
  /* Set Multisynth 0 to 112.5MHz using integer only mode (div by 4/6/8) */
  /* 25MHz * 36 = 900 MHz, then 900 MHz / 8 = 112.5 MHz */
  
  //Serial.println("Set PLLA to 900MHz"); //clockgen.setupPLLInt(SI5351_PLL_A, 36);
  Serial.println("Set PLLA to 600MHz");
  clockgen.setupPLLInt(SI5351_PLL_A, 15);
  //Serial.println("Set Output #0 to 112.5MHz");  //clockgen.setupMultisynthInt(0, SI5351_PLL_A, SI5351_MULTISYNTH_DIV_8);
  Serial.println("Set Output #0 to 75MHz");  
  clockgen.setupMultisynthInt(0, SI5351_PLL_A, SI5351_MULTISYNTH_DIV_8);

  /* FRACTIONAL MODE --> More flexible but introduce clock jitter */
  /* Setup PLLB to fractional mode @616.66667MHz (XTAL * 24 + 2/3) */
  /* Setup Multisynth 1 to 13.55311MHz (PLLB/45.5) */
  clockgen.setupPLL(SI5351_PLL_B, 24, 2, 3);
  Serial.println("Set Output #1 to 13.553115MHz");  
  clockgen.setupMultisynth(1, SI5351_PLL_B, 45, 1, 2);
  
  /*
    clockgen.setupMultisynth(output, SI5351_PLL_x, div, n, d);
    For the PLL input, use either SI5351_PLL_A or SI5351_PLL_B
    The final frequency is equal to the PLL / (div + n/d)
    div can range from 4 to 900
    n can range from 0 to 1,048,575
    d can range from 1 to 1,048,575
    
    Additional R Divider
    If you need to divide even more, to get to the < 100 KHz frequencies, there's an additional R divider, that divides the output once more by a fixed number:
    clockgen.setupRdiv(output, SI5351_R_DIV_x);
    output is the clock output #
    
    The R divider can be any of the following:
    SI5351_R_DIV_1
    SI5351_R_DIV_2
    SI5351_R_DIV_4
    SI5351_R_DIV_8
    SI5351_R_DIV_16
    SI5351_R_DIV_32
    SI5351_R_DIV_64
    SI5351_R_DIV_128
  */

  /* Multisynth 2 is not yet used and won't be enabled, but can be */
  /* Use PLLB @ 616.66667MHz, then divide by 900 -> 685.185 KHz */
  /* then divide by 64 for 10.706 KHz */
  /* configured using either PLL in either integer or fractional mode */

  Serial.println("Set Output #2 to 10.706 KHz");  
  clockgen.setupMultisynth(2, SI5351_PLL_B, 900, 0, 1);
  clockgen.setupRdiv(2, SI5351_R_DIV_64);
    
  /* Enable the clocks */
  clockgen.enableOutputs(true);
}

void loop(void) {  

}

