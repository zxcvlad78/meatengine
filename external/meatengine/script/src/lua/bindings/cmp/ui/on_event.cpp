#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/OnEvent.hpp>

static void bind(sol::state& lua) {
    lua.new_enum<me::cmp::ui::EventType>("EventType", {
        { "Hover", me::cmp::ui::EventType::Hover },
        { "Unhover", me::cmp::ui::EventType::Unhover },
        { "Press", me::cmp::ui::EventType::Press },
        { "Release", me::cmp::ui::EventType::Release },
        { "Click", me::cmp::ui::EventType::Click },
        { "Focus", me::cmp::ui::EventType::Focus },
        { "Blur", me::cmp::ui::EventType::Blur },
    });

    lua.new_usertype<me::cmp::ui::Event>("Event",
        sol::no_constructor,
        "type", &me::cmp::ui::Event::type,
        "local_pos", &me::cmp::ui::Event::local_pos,
        "target", &me::cmp::ui::Event::target,
        "handled", &me::cmp::ui::Event::handled
    );

    lua.new_usertype<me::cmp::ui::OnEvent>("OnEvent", sol::no_constructor);

    sol::table ui = lua["ui"].valid() ? lua["ui"].get<sol::table>() : lua.create_table();
    lua["ui"] = ui;

    ui.set_function("on_click",
        [](entt::registry& r, entt::entity e, sol::protected_function fn) {
            auto& h = r.get_or_emplace<me::cmp::ui::OnEvent>(e);
            h.cb = [fn](entt::registry& reg, entt::entity ent,
                        me::cmp::ui::Event& ev) mutable {
                if (ev.type != me::cmp::ui::EventType::Click) return;
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
            auto& h = r.get_or_emplace<me::cmp::ui::OnEvent>(e);
            h.cb = [fn](entt::registry& reg, entt::entity ent,
                        me::cmp::ui::Event& ev) mutable {
                if (ev.type != me::cmp::ui::EventType::Hover) return;
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