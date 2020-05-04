#pragma once

#include <stdint.h>

namespace sensors {
	enum : uint8_t {
		TYPE_SOIL = 1,
		TYPE_TEMP_HUM = 2,
		TYPE_TEMP = 3
	};

	// each sensor type has 18 bytes to use
	struct Soil {
		uint16_t moisture;
	};

	struct TempHum {
		uint16_t temperature;
		uint16_t humidity;
	};

	struct Temp {
		uint16_t temperature;
	};
}