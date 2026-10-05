#include <iostream>

#include <meatengine/meatengine.hpp>
#include <godlike/lua_bindings/common.hpp>
#include "GodLike.hpp"

int main() {
	godlike::lua_bindings::init(me::ScriptingServer::lua());

	auto main_font = me::ResourceLoader::load<me::Font>("res/fonts/mainfont.ttf");
	me::ResourceLoader::set_default<me::Font>(main_font);

	auto main_stylebox = me::ResourceLoader::load<me::StyleBox>("res/styleboxes/default.ttf");
	me::ResourceLoader::set_default<me::StyleBox>(main_stylebox);

	me::MainLoop mainloop("GodLike");

	mainloop.run(std::make_unique<GodLike>());

	return 0;
}