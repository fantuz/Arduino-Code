#include <OneWire.h>

/*
 * DS2408 8-Channel Addressable Switch
 * Writte by Glenn Trewitt, glenn at trewitt dot org
 * Some notes about the DS2408:
 *   - Unlike most input/output ports, the DS2408 doesn't have mode bits to
 *       set whether the pins are input or output.  If you issue a read command,
 *       they're inputs.  If you write to them, they're outputs.
 *   - For reading from a switch, you should use 10K pull-up resisters.
 */

void PrintBytes(uint8_t* addr, uint8_t count, bool newline=0) {
  for (uint8_t i = 0; i < count; i++) {
    Serial.print(addr[i]>>4, HEX);
    Serial.print(addr[i]&0x0f, HEX);
  }
  if (newline)
    Serial.println();
}

void ReadAndReport(OneWire* net, uint8_t* addr) {
  //Serial.print("  Reading DS2408 ");
  Serial.print("--- ");
  PrintBytes(addr, 16);
  Serial.println(" ---");

  //0xBE --- the second -- search into lib
  uint8_t buf[13];  // Put everything in the buffer so we can compute CRC easily.
  buf[0] = 0xF0;    // Read PIO Registers
  buf[1] = 0x88;    // LSB address
  buf[2] = 0x00;    // MSB address
  net->write_bytes(buf, 3);
  net->read_bytes(buf+3, 10);     // 3 cmd bytes, 6 data bytes, 2 0xFF, 2 CRC16
  net->reset();

  if (!OneWire::check_crc16(buf, 11, &buf[11])) {
    //Serial.print("CRC failure in DS2408 at ");
    //PrintBytes(addr, 8, true);
    Serial.println("CRC");
    //return;
  }

/*

1111111111111111
1111111111111111
1111111111111111
1111111111111111
1111111111111111
1010001001111110
01111011100010
0

--- 299163130000004A ---
0111111111111000
0010001000111111
1111111111111111
1010011101001100
1000111000111001
10


--- 289F27B804000031 ---
CRC

11111111111111111111111111111
3x cmdbytes
11111111
11111111
11111111

11111

                             6x data
                             11111111
                             11111111
                             11111111
                             11111111
                             11111111
                             11111111

                             2x 0xFF
                             11110100
                             01001111
                             
                             2x crc16
                             11001111
                             01110001
                             
                             00

--- 299163130000004A ---
                             01111111
                             11111000
                             00100010
                             
                             00111111
                             11111111
                             11111111
                             10100111
                             01001100
                             10001110
                             
                             00111001
                             10



--- 289F27B804000031 ---
CRC
11111111

11111111
11111111
11111111

11111111
11111111
11111111
11111111
11111111
11111111

10100010
01111110
01111011
1000100
(13+1)
--- 299163130000004A ---
0

11111111
11110000
01000100
01111111
11111111
11111111

01001110
10011001
00011100
011100110
(10+1)

*/

  //Serial.print("  DS2408 data = ");
  // First 3 bytes contain command, register address.
  //Serial.println(buf[3], BIN);
  // 3 cmd bytes, 6 data bytes, 2 0xFF, 2 CRC16
   //Serial.println("");
  Serial.println(buf[3], BIN);
  //Serial.println(buf[2], BIN);
  //Serial.println(buf[2], BIN);
//  Serial.print(buf[7], BIN);
//  Serial.print(buf[8], BIN);
//  Serial.print(buf[9], BIN);
  //Serial.println("");
  //Serial.println(buf[10], BIN);
//  Serial.print(buf[11], BIN);
//  Serial.print(buf[12], BIN);
//  Serial.print(buf[13], BIN);
//  //Serial.println("");
//  Serial.print(buf[14], BIN);
//  Serial.print(buf[15], BIN);
//  Serial.print(buf[16], BIN);
//  Serial.print(buf[17], BIN);
}

OneWire net(2);  // on pin 10

void setup(void) {
  Serial.begin(9600);
}

void loop(void) {
  byte i;
  byte present = 0;
  byte addr[8];
  Serial.println();
  
  if (!net.search(addr)) {
    //Serial.print("No more addresses.\n");
    net.reset_search();
    delay(6000);
    return;
  }
  
  if (OneWire::crc8(addr, 7) != addr[7]) {
    //Serial.print("CRC is not valid!\n");
    //return;
  }
  
  //if (addr[0] != 0x29) {
    //PrintBytes(addr, 8);
    //Serial.print(" is not a DS2408.\n");
    //return;
  //}

  ReadAndReport(&net, addr);
  Serial.println();
}
