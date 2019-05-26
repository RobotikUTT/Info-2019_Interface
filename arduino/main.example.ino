#include "can.hpp"


// this arduino is supposed to be pluged on the can bus. I communicates
// with the other arduino on the board through the hardware serial.
// It has to read the data from the other arduino and send it on the CAN bus.
// this arduino controls the pliers (a stepper, an AX12 and a limit switch)
// it also controls a L298 double H-bridge (aka the ball thrower)

#define SONAR_DELAY 100
#define EMERGENCY_SONAR_DELAY SONAR_DELAY

uint32_t orderMillis = millis();
uint32_t sonarMillis = millis();

CanHandler can;

void setup() {
	Serial.begin(57600);
	SPI.begin();
	Serial.println("begin");
	
	// Setup can
	can.setup();
}

void loop() {
	auto frame = can.read();

	if (can.is(frame, WHO_AM_I)) {
		// CODE HERE
	}

	// Flush data to send
	can.flush();
}