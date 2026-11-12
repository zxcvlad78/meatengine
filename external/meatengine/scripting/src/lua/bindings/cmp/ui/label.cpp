#include <meatengine/scripting/LuaBinder.hpp>
#include <meatengine/ui/cmp/Label.hpp>

static void bind(sol::state& lua) {
    lua.new_usertype<me::ui::cmp::Label>("Label",
        sol::constructors<me::ui::cmp::Label()>(),

        "text", sol::property(
            [](me::ui::cmp::Label& l) -> std::string {
                if (!l.text) return "";
                return l.text->getString().toAnsiString();
            },
            [](me::ui::cmp::Label& l, const std::string& s) {
                if (!l.text) return;
                l.text->setString(sf::String::fromUtf8(s.begin(), s.end()));
                l.dirty = true;
            }
        ),

        "font", sol::property(
            [](me::ui::cmp::Label& l) -> me::Font* {
                auto h = l.get_font().handle();
                return h ? h.get() : nullptr;
            },
            [](me::ui::cmp::Label& l, me::Font& f) {
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
            [](me::ui::cmp::Label& l) -> unsigned int {
                return l.text ? l.text->getCharacterSize() : 0u;
            },
            [](me::ui::cmp::Label& l, unsigned int s) {
                l.set_character_size(s);
            }
        ),

        "color", sol::property(
            [](me::ui::cmp::Label& l) -> sf::Color {
                return l.text ? l.text->getFillColor() : sf::Color::White;
            },
            [](me::ui::cmp::Label& l, sf::Color c) {
                if (l.text) l.text->setFillColor(c);
            }
        ),

        "position", sol::property(
            [](me::ui::cmp::Label& l) -> sf::Vector2f {
                return l.text ? l.text->getPosition() : sf::Vector2f{};
            },
            [](me::ui::cmp::Label& l, sf::Vector2f p) {
                if (l.text) l.text->setPosition(p);
            }
        ),

        "dirty", &me::ui::cmp::Label::dirty
    );

    me::lua::register_component<me::ui::cmp::Label>("Label", lua);
}

ME_LUA_BINDING(
    "engine:cmp:ui:Label",
    static_cast<unsigned>(me::lua::BindPriority::Component),
    &bind
)