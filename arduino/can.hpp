#include <stdint.h>
#include <mcp2515.h>
#include <Arduino.h>

#include <queue>
#include <vector>

#include "protocol.gen.hpp"

struct can_frame canRxMsg;  //strcture de la trame CAN reçue
struct can_frame canTxMsg; //trame CAN à envoyer

class CanHandler
{

private:
	std::queue<uint8_t> buffer;
	MCP2515* mcp2515;

public:
	void setup() {
		mcp2515 = new MCP2515(PIN_SPI_SS);

		mcp2515->reset();
		mcp2515->setBitrate(CAN_500KBPS, MCP_16MHZ);  // paramètres les plus rapides testés pour le CAN

		mcp2515->setNormalMode();
	}

	/**
	 * Read a frame data and decode it's arguments
	 * Returns an empty vector if no frame is fetched
	 */
	std::vector<int> read() {
		std::vector<int> data;
		MCP2515::ERROR can_error = mcp2515->readMessage(&canRxMsg);

		// If no error in message
		if (can_error == MCP2515::ERROR_OK) {

			// Find frame
			for (int j = 0; j < FRAMES_LENGTH; j++) {
				// If right id
				if (FRAMES[j][0] == canRxMsg.data[0]) {
					auto frame = FRAMES[j];
					int argCount = frame[1];
					int argIndex = 0;

					// Set id as first argument
					data.push_back(canRxMsg.data[0]);

					// On récupère les données
					for (int i = 1; i < canRxMsg.can_dlc && argIndex < argCount; i ++) {
						// 1-byte long
						if (frame[argIndex + 2] == 1) {
							data.push_back(canRxMsg.data[i]);

						// 2-byte long
						} else if (frame[argIndex + 2] == 2) {
							data.push_back(canRxMsg.data[i + 1] + (canRxMsg.data[i] << 8));
							i += 1;
						}

						argIndex ++;
					}

					break;
				}
			}

		}

		return data;
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
	 * Send buffered data to CAN bus
	 */
	void flush() {
		can_frame canTxMsg;

		while (!buffer.empty()) {
			// Get frame size
			int8_t size = buffer.front();
			canTxMsg.can_dlc = size;
			canTxMsg.can_id = buffer.front() + 10;

			// Then frame data
			for (int8_t i = 0; i < size; i++) {
				canTxMsg.data[i] = buffer.front();
				buffer.pop();
			}
		}

		mcp2515->sendMessage(&canTxMsg);
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
		this->buffer.push(data.size());

		// Then frame data
		for (const auto& value : data) {
			this->buffer.push(value);
		}

		va_end(argv);
	}

};
