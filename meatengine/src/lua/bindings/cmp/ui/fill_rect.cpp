#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/cmp/ui/FillRect.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ui::FillRect>("FillRect",
        sol::constructors<me::cmp::ui::FillRect()>(),
        "foreground", &me::cmp::ui::FillRect::foreground,
        "dirty", &me::cmp::ui::FillRect::dirty,
        "shape", &me::cmp::ui::FillRect::shape,

        "stylebox", sol::property(
            [](me::cmp::ui::FillRect& fr) -> me::StyleBox* {
                auto h = fr.stylebox.handle();
                return h ? h.get() : nullptr;
            },
            [](me::cmp::ui::FillRect& fr, me::StyleBox& sb) {
                auto sp = me::ResourceLoader::find_handle(&sb);
                if (!sp) {
                    throw std::runtime_error(
                        "FillRect.stylebox: StyleBox not from ResourceLoader "
                        "(load it via ResourceLoader.load_stylebox first)"
                    );}
                
                fr.stylebox = entt::resource<me::StyleBox>{sp};
                fr.dirty = true;
            }
        )
    );

    me::lua::register_component<me::cmp::ui::FillRect>("FillRect", lua);
}

ME_LUA_BINDING(
    "engine:cmp:ui:FillRect",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)