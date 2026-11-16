#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/TextEditState.hpp>
#include <meatengine/ui/widgets/line_edit.hpp>
#include <meatengine/ui/widgets/text_edit.hpp>
#include <SFML/System/String.hpp>

namespace {

std::string to_utf8(const std::u32string& s) {
    sf::String str(s);
    auto u8 = str.toUtf8();
    return std::string(u8.begin(), u8.end());
}
std::u32string from_utf8(const std::string& s) {
    return sf::String::fromUtf8(s.begin(), s.end()).toUtf32();
}

void bind(sol::state& lua) {
    lua.new_usertype<me::ui::cmp::TextEditState>("TextEditState",
        sol::no_constructor,
        "value", sol::property(
            [](me::ui::cmp::TextEditState& st) { return to_utf8(st.value); },
            [](me::ui::cmp::TextEditState& st, const std::string& s) { st.set_value_utf8(s); st.cursor = st.value.size(); st.anchor = st.cursor; }
        ),
        "placeholder", sol::property(
            [](me::ui::cmp::TextEditState& st) { return to_utf8(st.placeholder); },
            [](me::ui::cmp::TextEditState& st, const std::string& s) { st.placeholder = from_utf8(s); st.dirty = true; }
        ),
        "multiline", &me::ui::cmp::TextEditState::multiline,
        "read_only", &me::ui::cmp::TextEditState::read_only,
        "password", &me::ui::cmp::TextEditState::password,
        "character_size", sol::property(
            [](me::ui::cmp::TextEditState& st) { return st.character_size; },
            [](me::ui::cmp::TextEditState& st, unsigned int v) { st.character_size = v; st.dirty = true; }
        ),
        "cursor", sol::property(
            [](me::ui::cmp::TextEditState& st) { return st.cursor; },
            [](me::ui::cmp::TextEditState& st, std::size_t v) { st.cursor = std::min(v, st.value.size()); st.dirty = true; }
        ),
        "select_all", &me::ui::cmp::TextEditState::select_all,
        "insert_text", [](me::ui::cmp::TextEditState& st, const std::string& s) { st.insert_text(from_utf8(s)); }
    );

    lua.new_usertype<me::ui::cmp::LineEdit>("LineEdit", sol::constructors<me::ui::cmp::LineEdit()>());
    lua.new_usertype<me::ui::cmp::TextEdit>("TextEdit", sol::constructors<me::ui::cmp::TextEdit()>());

    me::lua::register_component<me::ui::cmp::TextEditState>("TextEditState", lua);
    me::lua::register_tag<me::ui::cmp::LineEdit>("LineEdit", lua);
    me::lua::register_tag<me::ui::cmp::TextEdit>("TextEdit", lua);

    sol::table widgets = lua["ui"]["widgets"].valid() ? lua["ui"]["widgets"].get<sol::table>() : lua.create_table();
    if (!lua["ui"].valid()) lua["ui"] = lua.create_table();
    lua["ui"]["widgets"] = widgets;

    widgets.set_function("make_line_edit", sol::overload(
        [](entt::registry& r) { return me::ui::widgets::make_line_edit(r); },
        [](entt::registry& r, entt::entity parent) { return me::ui::widgets::make_line_edit(r, parent); },
        [](entt::registry& r, entt::entity parent, const std::string& text) { return me::ui::widgets::make_line_edit(r, parent, text); }
    ));
    widgets.set_function("make_text_edit", sol::overload(
        [](entt::registry& r) { return me::ui::widgets::make_text_edit(r); },
        [](entt::registry& r, entt::entity parent) { return me::ui::widgets::make_text_edit(r, parent); },
        [](entt::registry& r, entt::entity parent, const std::string& text) { return me::ui::widgets::make_text_edit(r, parent, text); }
    ));
}

}

ME_LUA_BINDING(
    "engine:ui:cmp:TextEdit",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)

