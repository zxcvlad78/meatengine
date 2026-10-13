#include <meatengine/lua/LuaBinder.hpp>
#include <meatengine/cmp/ui/Label.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::cmp::ui::Label>("Label",
        sol::constructors<me::cmp::ui::Label()>(),

        "text", sol::property(
            [](me::cmp::ui::Label& l) -> std::string {
                if (!l.text) return "";
                return l.text->getString().toAnsiString();
            },
            [](me::cmp::ui::Label& l, const std::string& s) {
                if (!l.text) return;
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
                        "Label.font not from ResourceLoader "
                        "(load it via ResourceLoader.load_Font first)"
                    );
                }
                l.set_font(f);
            }
        ),

        "character_size", sol::property(
            [](me::cmp::ui::Label& l) -> unsigned int {
                return l.text ? l.text->getCharacterSize() : 0u;
            },
            [](me::cmp::ui::Label& l, unsigned int s) {
                if (l.text) l.text->setCharacterSize(s);
            }
        ),

        "color", sol::property(
            [](me::cmp::ui::Label& l) -> sf::Color {
                return l.text ? l.text->getFillColor() : sf::Color::White;
            },
            [](me::cmp::ui::Label& l, sf::Color c) {
                if (l.text) l.text->setFillColor(c);
            }
        ),

        "position", sol::property(
            [](me::cmp::ui::Label& l) -> sf::Vector2f {
                return l.text ? l.text->getPosition() : sf::Vector2f{};
            },
            [](me::cmp::ui::Label& l, sf::Vector2f p) {
                if (l.text) l.text->setPosition(p);
            }
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