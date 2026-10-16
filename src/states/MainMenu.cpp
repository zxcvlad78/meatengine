#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>
#include <meatengine/hierarchy.hpp>
#include <meatengine/PackedEntity.hpp>

#include <meatengine/cmp/Transform.hpp>
#include <meatengine/cmp/Parent.hpp>
#include <meatengine/cmp/Children.hpp>
#include <meatengine/cmp/ZIndex.hpp>
#include <meatengine/cmp/GlobalZIndex.hpp>

#include <meatengine/cmp/ui/UIRoot.hpp>
#include <meatengine/cmp/ui/Node.hpp>
#include <meatengine/cmp/ui/FlexStyle.hpp>
#include <meatengine/cmp/ui/FillRect.hpp>
#include <meatengine/cmp/ui/Label.hpp>
#include <meatengine/cmp/ui/BackgroundColor.hpp>
#include <meatengine/cmp/ui/BorderColor.hpp>
#include <meatengine/cmp/ui/BorderWidth.hpp>
#include <meatengine/cmp/ui/Interaction.hpp>
#include <meatengine/cmp/ui/FocusPolicy.hpp>
#include <meatengine/cmp/ui/Focusable.hpp>
#include <meatengine/cmp/ui/OnEvent.hpp>

#include <meatengine/meatnet/Network.hpp>

#include <godlike/states/MainMenu.hpp>

using namespace me;
using namespace me::cmp;
using namespace me::cmp::ui;

namespace godlike::states {

void MainMenu::on_enter(sf::RenderWindow&, entt::registry& r) {
    auto root = r.create();
    r.emplace<Transform>(root);
    r.emplace<Children>(root);
    r.emplace<UIRoot>(root);
    r.emplace<Node>(root, Node{
        .width = Val::Percent(100.f),
        .height = Val::Percent(100.f),
    });
    r.emplace<FlexStyle>(root, FlexStyle{
        .direction = FlexDirection::Column,
        .justify = JustifyContent::Center,
        .align_items = AlignItems::Center,
    });

    auto bg = PackedEntity::spawn("core:ui:panel", r);
    if (auto* n = r.try_get<Node>(bg)) {
        n->width = Val::Percent(100);
        n->height = Val::Percent(100);
        n->padding = UiRect::all(Val::Px(24.f));
    }
    hierarchy::set_parent(r, bg, root);

    auto title = PackedEntity::spawn("core:ui:title", r);
    hierarchy::set_parent(r, title, bg);

    auto host_btn = PackedEntity::spawn("core:ui:button", r);
    auto conn_btn = PackedEntity::spawn("core:ui:button", r);
    auto mods_btn = PackedEntity::spawn("core:ui:button", r);

    if (mods_btn == entt::null || host_btn == entt::null || conn_btn == entt::null) {
        std::cerr << "[MainMenu] failed to spawn buttons\n";
        return;
    }

    hierarchy::set_parent(r, host_btn, bg);
    hierarchy::set_button_text(r, host_btn, "Host");

    hierarchy::set_parent(r, conn_btn, bg);
    hierarchy::set_button_text(r, conn_btn, "Connect");

    hierarchy::set_parent(r, mods_btn, bg);
    hierarchy::set_button_text(r, mods_btn, "Mods");

    for (auto btn : { mods_btn, host_btn, conn_btn }) {
        if (auto* n = r.try_get<Node>(btn)) {
            n->width = Val::Percent(25.f);
            n->height = Val::Px(40.f);
        }
        if (auto* fs = r.try_get<FlexStyle>(btn))
            fs->flex_grow = 0.f;
    }

    r.emplace<OnEvent>(host_btn).cb =
        [](entt::registry&, entt::entity, Event& ev) {
            if (ev.type == EventType::Click) {
                me::net::Network::start_server(2302);
                ScriptedStateRegistry::change_to("core:world");
                ev.handled = true;
            }
        };

    r.emplace<OnEvent>(conn_btn).cb =
        [](entt::registry&, entt::entity, Event& ev) {
            if (ev.type == EventType::Click) {
                me::net::Network::connect("127.0.0.1:2302");
                ScriptedStateRegistry::change_to("core:world");
                ev.handled = true;
            }
        };

    r.emplace<OnEvent>(mods_btn).cb =
        [](entt::registry&, entt::entity, Event& ev) {
            if (ev.type == EventType::Click) {
                GameStateRegistry::change_to("godlike:mod_list");
                ev.handled = true;
            }
        };
}

void MainMenu::handle_event(sf::RenderWindow&, entt::registry&, const sf::Event&) {}

void MainMenu::on_exit(sf::RenderWindow&, entt::registry&) {}

}

ME_REGISTER_GAME_STATE("godlike:main_menu", godlike::states::MainMenu)