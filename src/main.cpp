#include <iostream>

#include <meatengine/app/App.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <meatengine/ecs/SystemRegistry.hpp>
#include <godlike/states/MainMenu.hpp>
#include <godlike/states/ModList.hpp>


int main() {
	auto app = me::App("GodLike").run();
	me::MainLoop::run(std::make_unique<godlike::states::MainMenu>());

	return 0;
}