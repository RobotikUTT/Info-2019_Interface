#include <stdint.h>
#include "mcp2515.h"
#include "Arduino.h"

/* remplaced by ArduinoSTL :
#include <array>
#include <queue>
#include <vector>
*/
#include <ArduinoSTL.h>

#include "protocol.gen.hpp"

struct can_frame canRxMsg;  //strcture de la trame CAN reçue
struct can_frame canTxMsg; //trame CAN à envoyer

class CanHandler
{
public:
	void setup() {
		mcp2515 = new MCP2515(SPI_SS_PIN);
		MCP2515::ERROR can_error;

		mcp2515->reset();
		mcp2515->setBitrate(CAN_500KBPS, MCP_16MHZ);  // paramètres les plus rapides testés pour le CAN

		can_error = mcp2515->setNormalMode();
	}

	/**
	 * Read a frame data and decode it's arguments
	 */
	std::vector<int> read() {
		MCP2515::ERROR can_error = mcp2515->readMessage(&canRxMsg);
		std::vector<int> data;

		if (canRxMsg.can_id == ARDUINO_CAN_ADDR || canRxMsg.can_id == ALL_CAN_ADDR) {
			// Si le CAN est disponible
			if (can_error == MCP2515::ERROR_OK) {
				int argIndex = 0;
				int argCount = canRxMsg.data[0];
				const uint8_t[] frame;

				// Find frame
				for (int j = 0; j < FRAMES_LENGTH; j++) {
					// If right id
					if (FRAMES[j][0] == canRxMsg.data[0]) {
						frame = FRAMES[j];
						break;
					}
				}

				// On récupère les données
				for (int i = 0; i < canRxMsg.can_dlc && argIndex < argCount; i ++) {
					// 1-byte long
					if (frame[argIndex + 2] == 1) {
						data.push_back(canRxMsg.data[i]);

					// 2-byte long
					} else if (frame[argIndex + 2] == 2) {
						data.push_back(canRxMsg.data[i + 1] + canRxMsg.data[i] << 8);
						i += 1;
					}

					argIndex ++;
				}
			}
		}

		return data;
	}

	/**
	 * Return whether the frame data belong to given frame
	 */
	bool is(std::vector<int> data, uint8_t[] frame) {
		return data.front() == frame[0];
	}

	/**
	 * Send buffered data to CAN bus
	 */
	void flush() {
		can_frame canTxMsg;
		canTxMsg.can_id = BBB_CAN_ADDR;

		while (!buffer.empty()) {
			// Get frame size
			int8_t size = buffer.pop_front();
			canTxMsg.can_dlc = size;

			// Then frame data
			for (int8_t i = 0; i < size; i++) {
				canTxMsg.data[i] = buffer.pop();
			}
		}

		mcp2515->sendMessage(&canTxMsg);
	}

	/**
	 * Format CAN frame data then add to buffer frames to send.
	 *
	 * Usage : send(SOME_FRAME_IN_PROTOCOL, framearg1, framearg2)
	 */
	void send(uint8_t[] mode, ...) {
		// Get all args
		va_list argv;
		va_start(argv, mode);

		int value;
		uint8_t length = mode[1];
		std::vector<uint8_t> data;

		// Set frame type first
		data.push_back(mode[0]);

		// Then copy args
		for (int8_t i = 0; i < length; i++) {
			value = va_arg(argv, int);

			// Set args according to their size
			if (mode[i + 2] == 1) {
				data.push_back(value);
			} else if (mode[i + 2] == 2) {
				data.push_back(value >> 8);
				data.push_back(value & 0x00FF);
			}
		}

		// Then push data to buffer by first giving it's size
		this->buffer.push_back(data.size());

		// Then frame data
		for (const auto& value : data) {
			this->buffer.push_back(value);
		}

		va_end(argv);
	}

private:
	std::queue<uint8_t> buffer;
};
