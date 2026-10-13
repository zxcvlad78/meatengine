#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>
#include <meatengine/ModLoader.hpp>
#include <meatengine/hierarchy.hpp>
#include <meatengine/PackedEntity.hpp>

#include <meatengine/cmp/Transform.hpp>
#include <meatengine/cmp/Children.hpp>
#include <meatengine/cmp/GlobalZIndex.hpp>

#include <meatengine/cmp/ui/UIRoot.hpp>
#include <meatengine/cmp/ui/Node.hpp>
#include <meatengine/cmp/ui/FlexStyle.hpp>
#include <meatengine/cmp/ui/FillRect.hpp>
#include <meatengine/cmp/ui/Label.hpp>
#include <meatengine/cmp/ui/BackgroundColor.hpp>
#include <meatengine/cmp/ui/BorderColor.hpp>
#include <meatengine/cmp/ui/BorderWidth.hpp>
#include <meatengine/cmp/ui/OnEvent.hpp>
#include <meatengine/cmp/ui/Focusable.hpp>

#include <godlike/states/ModList.hpp>

using namespace me;
using namespace me::cmp;
using namespace me::cmp::ui;

namespace godlike::states {

void ModList::on_enter(sf::RenderWindow&, entt::registry& r) {
    auto root = r.create();
    r.emplace<Transform>(root);
    r.emplace<Children>(root);
    r.emplace<UIRoot>(root);
    r.emplace<Node>(root, Node{
        .width = Val::Percent(100.f),
        .height = Val::Percent(100.f),
    });
    r.emplace<FlexStyle>(root, FlexStyle{
        .direction = FlexDirection::Row,
        .justify = JustifyContent::Center,
        .align_items = AlignItems::Stretch,
    });

    auto panel = PackedEntity::spawn("core:ui:panel", r);
    if (auto* n = r.try_get<Node>(panel)) {
        n->width = Val::Percent(50.f);
        n->height = Val::Percent(100.f);
        n->padding = UiRect::all(Val::Px(20.f));
    }
    hierarchy::set_parent(r, panel, root);

    auto descriptioin_panel = PackedEntity::spawn("core:ui:panel", r);
    if (auto* n = r.try_get<Node>(descriptioin_panel)) {
        n->width = Val::Percent(50.f);
        n->height = Val::Percent(100.f);
        n->padding = UiRect::all(Val::Px(20.f));
    }
    hierarchy::set_parent(r, descriptioin_panel, root);

    auto title = PackedEntity::spawn("core:ui:title", r);
    if (auto* l = r.try_get<Label>(title)) {
        l->set_string("Mods");
    }
    hierarchy::set_parent(r, title, panel);

    for (const auto& mi : ModLoader::list()) {
        auto row = PackedEntity::spawn("core:ui:button", r);
        if (row == entt::null) continue;

        hierarchy::set_button_text(r, row, mi.name + " " + mi.version.to_string());
        hierarchy::set_parent(r, row, panel);

        if (auto* n = r.try_get<Node>(row)) {
            n->width = Val::Percent(100.f);
            n->height = Val::Px(44.f);
        }

        r.emplace<OnEvent>(row).cb =
            [](entt::registry&, entt::entity, Event&) {};
    }

    auto back_btn = PackedEntity::spawn("core:ui:button", r);
    hierarchy::set_button_text(r, back_btn, "Back");
    if (back_btn != entt::null) {
        hierarchy::set_parent(r, back_btn, panel);
        if (auto* n = r.try_get<Node>(back_btn)) {
            n->width  = Val::Px(120.f);
            n->height = Val::Px(40.f);
        }
        if (auto* fs = r.try_get<FlexStyle>(back_btn))
            fs->align_self = AlignSelf::Center;

        r.emplace<OnEvent>(back_btn).cb =
            [](entt::registry&, entt::entity, Event& ev) {
                if (ev.type == EventType::Click) {
                    GameStateRegistry::change_to("godlike:main_menu");
                    ev.handled = true;
                }
            };
    }
}

void ModList::handle_event(sf::RenderWindow&, entt::registry&, const sf::Event&) {}
void ModList::on_exit(sf::RenderWindow&, entt::registry&) {}

} // namespace godlike::states

ME_REGISTER_GAME_STATE("godlike:mod_list", godlike::states::ModList)