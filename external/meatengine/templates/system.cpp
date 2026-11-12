#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>

//#include <meatengine/<module>/cmp/.hpp>


namespace me {
    static void update() {
        auto& reg = MainLoop::get_registry();
        auto& window = MainLoop::get_window();
        float dt = MainLoop::get_dt();

        // logic
    }
}

ME_REGISTER_SYSTEM(
    "namespace:name",
    me::SystemPhase::Update,
    0,
    me::update
)
