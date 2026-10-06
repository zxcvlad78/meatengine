#pragma once
#include <SFML/Graphics/Transform.hpp>

namespace godlike::components {
	struct MoveSpeed {
		float value = 50.f;
	};
	
	struct PlayerInput { };

	struct Satisfaction { float value = 0.f; };


	struct Container {

	};

	struct EnergyStorage {
		float capacity = 0.f;
    	float current = 0.f;
	};

	struct EnergyConsumer {
		float consumption;
		float min_input;
	};

	struct EnergyProducer {
		float production;
		float max_output;
	};
}