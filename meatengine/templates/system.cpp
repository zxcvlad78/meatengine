#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

//#include <meatengine/cmp/.hpp>


namespace me {
    static void update() {
        auto& reg = MainLoop::get_registry();
        auto& window = MainLoop::get_window();
        float dt = MainLoop::get_dt();

        // logic
    }
}

ME_REGISTER_SYSTEM(
    "engine:NAME:update",
    me::SystemPhase::Update,
    0,
    me::update
)
