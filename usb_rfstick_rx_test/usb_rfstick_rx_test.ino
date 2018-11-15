#include <Ports.h>
#include <RF12.h>
#include <avr/sleep.h>
void setup () {
	
	Serial.begin(115200);
	Serial.println("\n[RF USB Rrx test]");
	rf12_initialize(20, RF12_433MHZ, 4); //node,band,group
	pinMode(7, OUTPUT);
	digitalWrite(7, HIGH);	// set the LED off

}

void loop () {
	byte len;
	if (rf12_recvDone() && rf12_crc == 0) {
		// process incoming data here
		digitalWrite(7, LOW);	// set the LED on /* print the group */
		Serial.print(" grp: ");
		Serial.print(rf12_buf[0], HEX); /* print the node */
		Serial.print(" node: ");
		Serial.print((rf12_buf[1] & RF12_HDR_MASK),DEC);
		/* print the length of payload */
		Serial.print(" len: ");
		Serial.print(rf12_buf[2], DEC);
		/* print the content of payload */
		Serial.print(" data");
		for (len=3; len < rf12_buf[2]+3;len++){
			Serial.print(":");
			Serial.print(rf12_buf[len],HEX);
		}
		Serial.print(" ASCII:");
		for (len=3; len < rf12_buf[2]­3;len++){
			Serial.print((char )(rf12_buf[len]));
		}
		Serial.println();
		digitalWrite(7, HIGH);	// set the LED off
		/* send back an ack if required */
		if (RF12_WANTS_ACK) {
			digitalWrite(7, LOW); // set the RED LED off
			rf12_sendStart(RF12_ACK_REPLY, &len, 1);
			digitalWrite(7, HIGH); // set the RED LED off
		}
	}
}

