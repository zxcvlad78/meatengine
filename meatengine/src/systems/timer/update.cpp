#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/Timer.hpp>


namespace me {
    static void update() {
        auto& reg = MainLoop::get_registry();
        float dt = MainLoop::get_dt();

        for (auto [e, t] : reg.view<cmp::Timer>().each()) {
            if (t.paused) continue;
            if (t.time_left > 0.f) {
                t.time_left -= dt;
            } else {
                t.timeout_func(reg);
                if (!t.one_shot) {
                    t.start();
                } else t.paused = true;
            }
        }
    }
}

ME_REGISTER_SYSTEM(
    "engine:timer:update",
    me::SystemPhase::Update,
    std::numeric_limits<int>::min() + 2,
    me::update
)
