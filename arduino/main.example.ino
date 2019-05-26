#include "arduino/can.hpp"

CanHandler can;

void interrupts() {
	auto frame = can.read();

	// Frame invalide ou erreur recue
	if (!can.isValid(frame)) {
		return;
	}

	// Gestion des interruptions
	if (can.is(frame, WHO_AM_I)) {
		// CODE HERE
	}
}

void setup() {
	// Setup can
	can.setup();
	attachInterrupt(0, interrupts, FALLING);
}

void loop() {
	// Flush data to send
	can.flush();
}