#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/Timer.hpp>


namespace me {
    static void update() {

    }
}

ME_REGISTER_SYSTEM(
    "engine:sprite:update",
    me::SystemPhase::Update,
    std::numeric_limits<int>::min() + 5,
    me::update
)
