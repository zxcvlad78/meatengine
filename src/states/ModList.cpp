#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>
#include <meatengine/ModLoader.hpp>
#include <godlike/states/ModList.hpp>

struct BGFillRect {};
struct CButton {};

namespace godlike {
    namespace states {
        void ModList::on_enter(sf::RenderWindow& window, entt::registry& registry) {
            {auto e = registry.create();
                registry.emplace<me::cmp::ZIndex>(e, -1);
                auto& transform = registry.emplace<me::cmp::Transform>(e);

                auto& fr = registry.emplace<me::cmp::ui::FillRect>(e);
                fr.stylebox = me::ResourceLoader::load<me::StyleBox>("data/core/res/styleboxes/default.json");
                fr.shape.setSize({400.f, 500.f});
                fr.foreground = false;
                registry.emplace<BGFillRect>(e);
            }

            auto spawn_info = [&](const std::string& text) {
                auto info = me::PackedEntity::spawn("core:ui:button", registry);
                registry.emplace<CButton>(info);

                auto* fr = registry.try_get<me::cmp::ui::FillRect>(info);
                auto* label = registry.try_get<me::cmp::ui::Label>(info);

                if (label != nullptr) {
                    label->text->setString(text);

                    if (fr != nullptr) {
                        auto new_size = label->text->getLocalBounds().size * 1.2f;
                        auto fr_size = fr->shape.getSize();
                        std::cout << "{" << fr_size.x << ", " << fr_size.y << "}" << " : ";
                        std::cout << "{" << new_size.x << ", " << new_size.y << "}" << std::endl;
                        if (new_size.x > fr_size.x)
                            fr_size.x = new_size.x;
                        if (new_size.y > fr_size.y)
                            fr_size.y = new_size.y;
                        fr->shape.setSize(fr_size);
                    }
                }

                return info;
            };
            
            for (const auto& mi : me::ModLoader::list()) {
                spawn_info(mi.name + "\n" + mi.version.to_string());
            }
        }

        void ModList::handle_event(sf::RenderWindow& window, entt::registry& registry, const sf::Event& event)  {
            // handle event))
        }

        void ModList::on_exit(sf::RenderWindow& window, entt::registry& registry)  {
            // game state exit)00
        }
    }
}

static void update()  {
    auto& reg = me::MainLoop::get_registry();
    auto& window = me::MainLoop::get_window();
    
    const sf::Vector2f win_size(window.getSize());

    for (auto [e, fr] : reg.view<me::cmp::ui::FillRect, BGFillRect>().each()) {
        fr.shape.setSize(win_size);
    }

    static constexpr float spacing = 20.f;
    int items_count = 0;
    float total_height = 0.f;

    auto button_view = reg.view<me::cmp::Transform, me::cmp::ui::FillRect, CButton>();
    
    for (auto [e, t, fr] : button_view.each()) {
        total_height += fr.shape.getSize().y + spacing;
        items_count++;
    }
    
    if (items_count > 0) {
        total_height -= spacing;
    }

    float current_y = spacing;

    for (auto [e, t, fr] : button_view.each()) {
        t.position.x = spacing; //(win_size.x - fr.shape.getSize().x) / 2.f;
        t.position.y = current_y;

        current_y += fr.shape.getSize().y + spacing;
    }
}




ME_REGISTER_GAME_STATE("godlike:mod_list", godlike::states::ModList)
ME_REGISTER_SYSTEM(
    "godlike:states:mod_list:update",
    me::SystemPhase::Update,
    0,
    update
)