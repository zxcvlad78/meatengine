#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <meatengine/app/GameStateRegistry.hpp>
#include <meatengine/hierarchy/hierarchy.hpp>
#include <meatengine/ecs/PackedEntity.hpp>
#include <meatengine/assets/ConfigFile.hpp>
#include <meatengine/scripting/ScriptedStateRegistry.hpp>

#include <meatengine/spatial/cmp/Transform.hpp>
#include <meatengine/hierarchy/cmp/Parent.hpp>
#include <meatengine/hierarchy/cmp/Children.hpp>
#include <meatengine/abstract/cmp/ZIndex.hpp>
#include <meatengine/abstract/cmp/GlobalZIndex.hpp>

#include <meatengine/ui/helpers.hpp>
#include <meatengine/ui/cmp/UIRoot.hpp>
#include <meatengine/ui/cmp/Node.hpp>
#include <meatengine/ui/cmp/FlexStyle.hpp>
#include <meatengine/ui/cmp/FillRect.hpp>
#include <meatengine/ui/cmp/Label.hpp>
#include <meatengine/ui/cmp/BackgroundColor.hpp>
#include <meatengine/ui/cmp/BorderColor.hpp>
#include <meatengine/ui/cmp/BorderWidth.hpp>
#include <meatengine/ui/cmp/Interaction.hpp>
#include <meatengine/ui/cmp/FocusPolicy.hpp>
#include <meatengine/ui/cmp/Focusable.hpp>
#include <meatengine/ui/cmp/OnEvent.hpp>

#include <meatengine/ui/widgets/button.hpp>
#include <meatengine/ui/widgets/panel.hpp>
#include <meatengine/ui/widgets/label.hpp>

#include <meatengine/net/Network.hpp>

#include <godlike/states/MainMenu.hpp>

using namespace me;
using namespace me::spatial::cmp;
using namespace me::hierarchy::cmp;
using namespace me::abstract::cmp;
using namespace me::ui::cmp;

ConfigFile* config = new ConfigFile("user://states/main_menu.cfg");
std::string connection_address;

void load_cfg() {
    connection_address = "127.0.0.1:2302";
    return;
    config->load();
}

namespace godlike::states {

void MainMenu::on_enter(sf::RenderWindow&, entt::registry& r) {
    load_cfg();

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
    ui::set_label_text(r, title, "Godlike");

    auto host_btn = PackedEntity::spawn("core:ui:button", r);
    auto conn_btn = PackedEntity::spawn("core:ui:button", r);
    auto mods_btn = PackedEntity::spawn("core:ui:button", r);

    if (mods_btn == entt::null || host_btn == entt::null || conn_btn == entt::null) {
        std::cerr << "[MainMenu] failed to spawn buttons\n";
        return;
    }

    hierarchy::set_parent(r, host_btn, bg);
    ui::set_text(r, host_btn, "Host");

    hierarchy::set_parent(r, conn_btn, bg);
    ui::set_text(r, conn_btn, "Connect");

    hierarchy::set_parent(r, mods_btn, bg);
    ui::set_text(r, mods_btn, "Mods");

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
                me::net::Network::set_batching(true);
                ScriptedStateRegistry::change_to("core:world");
                ev.handled = true;
            }
        };

    r.emplace<OnEvent>(conn_btn).cb =
        [](entt::registry&, entt::entity, Event& ev) {
            if (ev.type == EventType::Click) {
                ev.handled = true;
                me::net::Network::set_batching(true);
                if (!me::net::Network::connect(connection_address)) return;
                ScriptedStateRegistry::change_to("core:world");
            }
        };

    r.emplace<OnEvent>(mods_btn).cb =
        [](entt::registry&, entt::entity, Event& ev) {
            if (ev.type == EventType::Click) {
                ev.handled = true;
                GameStateRegistry::change_to("godlike:mod_list");
            }
        };
}

void MainMenu::handle_event(sf::RenderWindow&, entt::registry&, const sf::Event&) {}

void MainMenu::on_exit(sf::RenderWindow&, entt::registry&) {}

}

ME_REGISTER_GAME_STATE("godlike:main_menu", godlike::states::MainMenu)
