/*
 By Roee Bloch  RFM69HW to control 2 lights ON/OFF each press is toggle
 All right Reserved
 Copyright (c) 2015 All Right Reserved, http://www.electronics-freak.com

 This source is subject to the Roee Bloch License.
 Please see the License.txt file for more information.
 All other rights reserved.

 THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 This code is based on the code below, I have just changed it according to my needs

The documentation is not complete, so in order to complete this project you have to take care of the following:
1. RFM69HW need antenna – so connect wire according to your frequency of board (in case of 433Mhz = 17 cm wire)
2. This module SPI signals and board is 3.3V, so use shifting level (preferred resistors only!)
3. In Transmit mode the power is high, so you should use 3.3V 500mA power supply, the Nano power is not good enough)
4. Connect on 3.3V Tantalum capacitor of 100uF (to ensure power when transmitting)
5. connect module DIO0 to Arduino D2 (this is the interrupt in that you should use in RXTXBlinky program)

SPI PINS Connections:
NANO –> RFM69HW
D10 —–> NSS
D11 —–> MOSI
D12 —–> MISO
D13 —–> SCK
GND —–> GND
—————— 3.3V —> connect external 3.3V
D2  <---DIO0

D6-> ON/OFF switch  -> GND
We read D6 if low we toggle LED1 if High we toggle LED2

Push button switch between GND to D3 -> this is an interrupt to save power while not need to send anything


*/

// ***************************************************************************************
// Sample RFM69 sketch for Moteino to illustrate sending and receiving, button interrupts
// ***************************************************************************************
// When you press the button on the SENDER Moteino, it will send a short message to the
// RECEIVER Moteino and wait for an ACK (acknowledgement that message was received) from
// the RECEIVER Moteino. If the ACK was received, the SENDER will blink the onboard LED
// a few times. The RECEIVER listens to a specific token, and it alternates the onboard LED
// state from HIGH to LOW or vice versa whenever this token is received.
// ***************************************************************************************
// Hardware setup:
// ***************************************************************************************
// On the sender, hook up a momentary tactile button to D3 like this:
//          __-__
//        __|   |___
// GND ----> BTN ----> D3 (D11 on MoteinoMEGA)
// Load this sketch on the RECEIVER with NODEID=RECEIVER (adjust in config section below)
// Load this sketch on the SENDER with NODEID=SENDER (adjust in config section below)
// RFM69 library and code by Felix Rusu - felix@lowpowerlab.com
// Get libraries at: https://github.com/LowPowerLab/
// Make sure you adjust the settings in the configuration section below !!!
// **********************************************************************************
// Copyright Felix Rusu, LowPowerLab.com
// Library and code by Felix Rusu - felix@lowpowerlab.com
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it
// and/or modify it under the terms of the GNU General
// Public License as published by the Free Software
// Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE. See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General
// Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//
// Licence can be viewed at
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code
// **********************************************************************************

#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <LowPower.h> //get library from: https://github.com/lowpowerlab/lowpower

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID     100  //the same on all nodes that talk to each other
#define RECEIVER      1    //unique ID of the gateway/receiver
#define SENDER        2
#define NODEID        SENDER  //change to "SENDER" if this is the sender node (the one with the button)
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW    //uncomment only for RFM69HW! Remove/comment if you have RFM69W!
//*********************************************************************************************
#define SERIAL_BAUD   115200
#ifdef __AVR_ATmega1284P__
#define LED           13 // Moteino MEGAs have LEDs on D15 on NANO is 13
#define BUTTON_INT    1 //user button on interrupt 1 (D3)
#define BUTTON_PIN    11 //user button on interrupt 1 (D3)
#else
#define LED           9 // Moteinos have LEDs on D9
#define BUTTON_INT    1 //user button on interrupt 1 (D3)
#define BUTTON_PIN    3 //user button on interrupt 1 (D3)
#endif

#define LED_GREEN       4 //GREEN LED on the SENDER
#define LED_RED         5 //RED LED on the SENDER
#define RX_TOGGLE_PIN   7 //GPIO to toggle on the RECEIVER
#define MY_OUT 8 // roee
#define SW1SW2 6 // read pin if 0 LED1 if 1 LED2

RFM69 radio;
const void* mes1 = "S1"; // different messages to different leds
const void* mes2 = "S2"; // different messages to different leds
int myinput; //SW1SW2 state


void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("beginning receiver 2");
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  Serial.flush();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  attachInterrupt(BUTTON_INT, handleButton, FALLING);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(MY_OUT, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(RX_TOGGLE_PIN, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH);
  pinMode(SW1SW2, INPUT_PULLUP);
}

//******** THIS IS INTERRUPT BASED DEBOUNCING FOR BUTTON ATTACHED TO D3 (INTERRUPT 1)
#define FLAG_INTERRUPT 0x01
volatile int mainEventFlags = 0;
boolean buttonPressed = false;
void handleButton()
{
  mainEventFlags |= FLAG_INTERRUPT;
}

byte LEDSTATE = LOW; //LOW=0
void loop() {
  //******** THIS IS INTERRUPT BASED DEBOUNCING FOR BUTTON ATTACHED TO D3 (INTERRUPT 1)
  if (mainEventFlags & FLAG_INTERRUPT)
  {
    LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_ON);
    mainEventFlags &= ~FLAG_INTERRUPT;
    if (!digitalRead(BUTTON_PIN)) {
      buttonPressed = true;
    }
  }

  if (buttonPressed)
  {
    Serial.println("Button pressed!");
    buttonPressed = false;
    myinput = digitalRead(SW1SW2);
    Serial.print("Changing LED- value is    ");
    Serial.println(myinput);
    if (LEDSTATE == LOW)
    {
      LEDSTATE = HIGH;
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
    }
    else
    {
      LEDSTATE = LOW;
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, HIGH);
    }
    if (0 == myinput)
    {
      radio.sendWithRetry(RECEIVER, mes1, 2);
    } else
    {
      radio.sendWithRetry(RECEIVER, mes2, 2);
    }

    //target node Id, message as string or byte array, message length
    //     Blink(LED, 200, 10); //blink LED 3 times, 40ms between blinks

  }

  //check if something was received (could be an interrupt from the radio)
  if (radio.receiveDone())
  {
    //print message received to serial
    Serial.print('['); Serial.print(radio.SENDERID); Serial.print("] ");
    Serial.print((char*)radio.DATA);
    Serial.print("   [RX_RSSI:"); Serial.print(radio.RSSI); Serial.print("]");
    Serial.println();

    //check if received message is 2 bytes long, and check if the message is specifically "Hi"
    if (radio.DATALEN == 2 && radio.DATA[0] == 'S' && radio.DATA[1] == '1')
    {
      Serial.print("Inside 2");
      //      Blink(LED, 200, 2); //blink LED 3 times, 40ms between blinks
      Roee ();
      if (LEDSTATE == LOW)
        LEDSTATE = HIGH;
      else LEDSTATE = LOW;
      digitalWrite(LED, LEDSTATE);
      digitalWrite(RX_TOGGLE_PIN, LEDSTATE);
    }

    //check if sender wanted an ACK
    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
    }
  }

  radio.receiveDone(); //put radio in RX mode
  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON); //sleep Moteino in low power mode (to save battery)
}

void Blink(int PIN, int DELAY_MS, int loops)
{
  Serial.print("Inside Blink loop now ....");
  for (int i = 0; i < loops; i++)
  {
    digitalWrite(PIN, HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN, LOW);
    delay(DELAY_MS);
  }
}
void Roee()
{
  for (int x = 0; x < 2; x++)
  {
    digitalWrite(MY_OUT, HIGH);
    delay(100);
    digitalWrite(MY_OUT, LOW);
    delay(100);

  }

}

