#pragma once

#include <optional>
#include <meatengine/cmp/ui/States.hpp>
#include <meatengine/Resources.hpp>
#include <meatengine/ResourceLoader.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/entity.hpp>

namespace me::cmp::ui {
	struct Label {
	private:
		entt::resource<me::Font> _font{};
	public:
        bool dirty = true;
		std::optional<sf::Text> text;
        entt::resource<me::StyleBox> stylebox;

		entt::resource<me::Font> get_font() { return _font; }
        void set_font(entt::resource<me::Font> font) {
            if (!font) {
                _font = {};
                text.reset();
                return;
            }
            _font = std::move(font);
            text.emplace(_font->res);
			dirty = true;
        }

        void set_font(me::Font& font) {
            auto h = me::ResourceLoader::find_handle(&font);
            if (!h) {
                throw std::runtime_error("Label::set_font: Font not from ResourceLoader");
            }
            set_font(entt::resource<me::Font>{h});
        }


		void update_fill(entt::registry& reg, entt::entity e) {
            if (!dirty) return;
            if (!stylebox) return;
			dirty = false;			
            
			if (!text.has_value()) return;

            if (reg.all_of<Disabled>(e))
                text->setFillColor(stylebox->get_value<sf::Color>("font_disabled_color", sf::Color::White));
            else if (reg.all_of<Pressed>(e))
                text->setFillColor(stylebox->get_value<sf::Color>("font_pressed_color", sf::Color::White));
            else if (reg.all_of<Hovered>(e))
                text->setFillColor(stylebox->get_value<sf::Color>("font_hovered_color", sf::Color::White));
            else 
                text->setFillColor(stylebox->get_value<sf::Color>("font_color", sf::Color::White));
		}

		Label() = default; 
		Label(me::Font& font) : text(font.res) {
            if (!stylebox) {
                std::shared_ptr<me::StyleBox> def = ResourceLoader::get_default<me::StyleBox>();
                if (def != nullptr)
                    stylebox = entt::resource<me::StyleBox>{def};
            }
        }
	};
}