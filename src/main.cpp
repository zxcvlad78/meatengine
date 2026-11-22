#include <meatengine/app/App.hpp>
#include <meatengine/app/MainLoop.hpp>

#include <meatengine/ui/widgets/ui_root.hpp>
#include <meatengine/ui/widgets/label.hpp>

#include <godlike/states/MainMenu.hpp>

using namespace me;
using namespace godlike;

int main() {
    me::App("Godlike").run();
    me::MainLoop::run(std::make_unique<states::MainMenu>());
    return 0;
}
