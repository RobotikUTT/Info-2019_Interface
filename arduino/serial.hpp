#include <stdint.h>
#include <Arduino.h>

#include <queue>
#include <vector>

#include "protocol.gen.hpp"

class SerialHandler
{
public:
	void begin(unsigned long baud) {
		Serial.begin(baud);
	}

	/**
	 * Read a frame data and decode it's arguments
	 * Returns an empty vector if no frame is fetched
	 */
	std::vector<int> read() {
		std::vector<int> data;

		if (Serial.available() > 0) {
			// Read frame type
			int8_t frame_type = Serial.read();
			
			// Find frame
			for (int j = 0; j < FRAMES_LENGTH; j++) {
				// If right id
				if (FRAMES[j][0] == frame_type) {
					auto frame = FRAMES[j];
					int argCount = frame[1];

					// Set id as first argument
					data.push_back(frame_type);

					// Expect all args to follow up
					for (int argIndex = 0; argIndex < argCount; argIndex ++) {
						// 1-byte long
						if (frame[argIndex + 2] == 1) {
							data.push_back(readFromSerial());

						// 2-byte long
						} else if (frame[argIndex + 2] == 2) {
							int first = readFromSerial();
							int second = readFromSerial();

							data.push_back(second + (first << 8));
						}
					}

					break;
				}
			}
		}

		return data;
	}

	int readFromSerial() {
		while(Serial.available() == 0) {
			delay(10);
		}

		return Serial.read();
	}

	/**
	 * Return whether the frame data belong to given frame
	 */
	bool is(const std::vector<int> data, const uint8_t frame[]) {
		return data.size() > 0 && data.front() == frame[0];
	}

	/**
	 * Returns whether the frame contains enough data
	 */
	bool complete(const std::vector<int> data) {
		return data.size() > 0;
	}

	/**
	 * Format CAN frame data then add to buffer frames to send.
	 *
	 * Usage : send(SOME_FRAME_IN_PROTOCOL, framearg1, framearg2)
	 */
	void send(const uint8_t mode[], ...) {
		// Get all args
		va_list argv;
		va_start(argv, mode);

		int value;
		uint8_t length = mode[1];

		// Set frame type first
		Serial.write(mode[0]);

		// Then copy args
		for (int8_t i = 0; i < length; i++) {
			value = va_arg(argv, int);

			// Set args according to their size
			if (mode[i + 2] == 1) {
				Serial.write(value);
			} else if (mode[i + 2] == 2) {
				Serial.write(value >> 8);
				Serial.write(value & 0x00FF);
			}
		}

		va_end(argv);
	}

};
