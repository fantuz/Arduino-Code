//#include <WProgram.h>

#include <Arduino.h>
#include <DS2408.h>
#include <OneWire.h>
#include <string.h>
#include <stdio.h>

#define ONE_WIRE_BUS_PORT 2
DS2408  ds(ONE_WIRE_BUS_PORT);

Devices devices;
uint8_t device_count;

void setup(void) {
    Serial.begin(9600);

    device_count = ds.find(&devices);
    print_devices(&devices, device_count);
}

void print_byte(uint8_t data) {
    for(int index=0; index<8; index++) {
        Serial.print(data & 1, BIN);
        data = data >> 1;
    }
}

void print_address(byte* address) {
    Serial.print("[");
    for(int index = 0; index < sizeof(Device); index++) {
        Serial.print(address[index], HEX);
        if(index < sizeof(Device)-1)
            Serial.print(" ");
    }
    Serial.print("] ");
}

void print_devices(Devices* devices, uint8_t device_count) {
    Serial.print("Found ");
    Serial.print(device_count , HEX);
    Serial.println(" devices.");

    for(int index=0; index < device_count; index++) {
        Serial.print(index+1, HEX);
        Serial.print(". ");
        print_address((*devices)[index]);
        Serial.println();
    }
}

void loop(void) { 
    delay(15000);
}

