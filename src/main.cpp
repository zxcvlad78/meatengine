#include <iostream>

#include <meatengine/meatengine.hpp>
#include <meatengine/SystemRegistry.hpp>
#include <godlike/states/MainMenu.hpp>
#include <godlike/states/ModList.hpp>

int main() {
	me::MainLoop::init("GodLike");
	me::ModLoader::init();

	auto main_font = me::ResourceLoader::load<me::Font>("data/core/res/fonts/mainfont.ttf");
	me::ResourceLoader::set_default<me::Font>(main_font);
	
	auto main_stylebox = me::ResourceLoader::load<me::StyleBox>("data/core/res/styleboxes/default.json");
	me::ResourceLoader::set_default<me::StyleBox>(main_stylebox);

	me::MainLoop::run(std::make_unique<godlike::states::MainMenu>());

	return 0;
}