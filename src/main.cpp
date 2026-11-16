#include <iostream>

#include <meatengine/app/App.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <meatengine/audio/SoundPlayer.hpp>
#include <meatengine/ecs/SystemRegistry.hpp>
#include <godlike/states/MainMenu.hpp>
#include <godlike/states/ModList.hpp>
#include <godlike/packed_entities.hpp>
#include <meatengine/plugins/TextEditor.hpp>

int main() {
	me::App("GodLike")
		.add_plugins<me::TextEditorPlugin>()
		.run();
	me::MainLoop::run(std::make_unique<godlike::states::MainMenu>());
	me::SoundPlayer::reset();

	return 0;
}