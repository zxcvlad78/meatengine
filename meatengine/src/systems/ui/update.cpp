#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/ui/FillRect.hpp>
#include <meatengine/cmp/ui/Label.hpp>

namespace me {

static void update() {
    auto& reg = me::MainLoop::get_registry();

    for (auto [e, fr] : reg.view<cmp::ui::FillRect>().each()) {
        fr.update_fill(reg, e);
        
    }
    for (auto [e, label] : reg.view<cmp::ui::Label>().each()) {
        label.update_fill(reg, e);
        
    }

};

}

ME_REGISTER_SYSTEM(
    "engine:ui:update",
    me::SystemPhase::Update,
    std::numeric_limits<int>::min() + 9,
    me::update
)