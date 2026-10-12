#include <string>
#include <meatengine/lua/LuaBinder.hpp>

#include <meatengine/cmp/ui/Label.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ui::Label>("Label",
        "text", sol::property(
            [](me::cmp::ui::Label& l) -> std::string {
                return l.text->getString().toAnsiString();
            },
            [](me::cmp::ui::Label& l, const std::string& s) {
                l.text->setString(sf::String::fromUtf8(s.begin(), s.end()));
                l.dirty = true;
            }
        ),
        "font", sol::property(
            [](me::cmp::ui::Label& l) -> me::Font* {
                auto h = l.get_font().handle();
                return h ? h.get() : nullptr;
            },
            [](me::cmp::ui::Label& l, me::Font& f) {
                auto fh = me::ResourceLoader::find_handle(&f);
                if (!fh) {
                    throw std::runtime_error(
                        "Label.font not from ResourceLoader (load it via ResourceLoader.load_Font first)"
                    );}
                
                l.set_font(f);
            }
        ),
        "character_size", sol::property(
            [](me::cmp::ui::Label& l) { return l.text->getCharacterSize(); },
            [](me::cmp::ui::Label& l, unsigned int s) { l.text->setCharacterSize(s); }
        ),
        "color", sol::property(
            [](me::cmp::ui::Label& l) { return l.text->getFillColor(); },
            [](me::cmp::ui::Label& l, sf::Color c) { l.text->setFillColor(c); }
        ),
        "position", sol::property(
            [](me::cmp::ui::Label& l) { return l.text->getPosition(); },
            [](me::cmp::ui::Label& l, sf::Vector2f p) { l.text->setPosition(p); }
        ),
        "dirty", &me::cmp::ui::Label::dirty
    );

    me::lua::register_component<me::cmp::ui::Label>("Label", lua);
}

ME_LUA_BINDING(
    "engine:cmp:ui:Label",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)