#include <iostream>

#include <meatengine/meatengine.hpp>
#include <meatengine/SystemRegistry.hpp>
#include <godlike/states/MainMenu.hpp>
#include <godlike/states/ModList.hpp>


int main() {
	auto app = me::App("GodLike").run();
	me::MainLoop::run(std::make_unique<godlike::states::MainMenu>());

	return 0;
}