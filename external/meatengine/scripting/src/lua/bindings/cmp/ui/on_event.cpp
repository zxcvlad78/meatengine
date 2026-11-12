#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/OnEvent.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::ui::cmp::EventType>("EventType", {
        { "Hover", me::ui::cmp::EventType::Hover },
        { "Unhover", me::ui::cmp::EventType::Unhover },
        { "Press", me::ui::cmp::EventType::Press },
        { "Release", me::ui::cmp::EventType::Release },
        { "Click", me::ui::cmp::EventType::Click },
        { "Focus", me::ui::cmp::EventType::Focus },
        { "Blur", me::ui::cmp::EventType::Blur },
    });

    lua.new_usertype<me::ui::cmp::Event>("Event",
        sol::no_constructor,
        "type", &me::ui::cmp::Event::type,
        "local_pos", &me::ui::cmp::Event::local_pos,
        "target", &me::ui::cmp::Event::target,
        "handled", &me::ui::cmp::Event::handled
    );

    lua.new_usertype<me::ui::cmp::OnEvent>("OnEvent", sol::no_constructor);

    sol::table ui = lua["ui"].valid() ? lua["ui"].get<sol::table>() : lua.create_table();
    lua["ui"] = ui;

    ui.set_function("on_click",
        [](entt::registry& r, entt::entity e, sol::protected_function fn) {
            auto& h = r.get_or_emplace<me::ui::cmp::OnEvent>(e);
            h.cb = [fn](entt::registry& reg, entt::entity ent,
                        me::ui::cmp::Event& ev) mutable {
                if (ev.type != me::ui::cmp::EventType::Click) return;
                auto res = fn(ent);
                if (!res.valid()) {
                    sol::error err = res;
                    std::cerr << "[ui.on_click] " << err.what() << "\n";
                }
                ev.handled = true;
            };
        });

    ui.set_function("on_hover",
        [](entt::registry& r, entt::entity e, sol::protected_function fn) {
            auto& h = r.get_or_emplace<me::ui::cmp::OnEvent>(e);
            h.cb = [fn](entt::registry& reg, entt::entity ent,
                        me::ui::cmp::Event& ev) mutable {
                if (ev.type != me::ui::cmp::EventType::Hover) return;
                auto res = fn(ent);
                if (!res.valid()) {
                    sol::error err = res;
                    std::cerr << "[ui.on_hover] " << err.what() << "\n";
                }
            };
        });
}

ME_LUA_BINDING(
    "engine:ui:OnEvent",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)