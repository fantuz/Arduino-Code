#define DATAOUT 11//MOSI
#define DATAIN 12//MISO
#define SPICLOCK 13//sck
#define SLAVESELECT 10//ss
 
//opcodes
#define WREN 6
#define WRDI 4
#define RDSR 5
#define WRSR 1
#define READ 3
#define WRITE 2
 
byte eeprom_output_data;
byte eeprom_input_data=0;
byte clr;
int address=0;
 
char spi_transfer(volatile char data) {
SPDR = data; // Start the transmission
while (!(SPSR &amp; (1&lt; return SPDR; // return the received byte
}
 
void setup() {
Serial.begin(9600);
 
pinMode(DATAOUT, OUTPUT);
pinMode(DATAIN, INPUT);
pinMode(SPICLOCK,OUTPUT);
pinMode(SLAVESELECT,OUTPUT);
digitalWrite(SLAVESELECT,HIGH); //disable device
}
 
byte read_eeprom(int EEPROM_address) {
//READ EEPROM
int data;
digitalWrite(SLAVESELECT,LOW);
spi_transfer(READ); //transmit read opcode
spi_transfer((char)(EEPROM_address&gt;&gt;8)); //send MSByte address first
spi_transfer((char)(EEPROM_address)); //send LSByte address
data = spi_transfer(0xFF); //get data byte
digitalWrite(SLAVESELECT,HIGH); //release chip, signal end transfer
return data;
}
 
void loop() {
eeprom_output_data = read_eeprom(address);
//Serial.print("At 0x");
//Serial.print(address, HEX);
//Serial.print(" = ");
Serial.print("0x");
Serial.print(eeprom_output_data,HEX);
address++;
//when finished dump, enter infinite loop
if (address == 0x4000) {
Serial.println("};");
while(1);
}
Serial.print(", ");
if (!(address%20)) Serial.println("");
//delay(50); //pause for readability
}

