#pragma once
#include <string>

namespace godlike {
	struct ItemDefinition {
		std::string id;
		std::string icon_path;

		uint32_t stack_size = 1;
	};
}