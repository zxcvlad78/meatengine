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

std::string selected_mod;

std::string text_mod_info(const std::string& mod_name) {
    auto mi = me::ModLoader::find(mod_name);
    if (!mi) return "";

    std::string s;
    if (!mi->title.empty())
        s.append(mi->title + " (" + mi->name + ")" + "\n");
    else s.append(mi->name + "\n");

    s.append(mi->description + "\n\n");
    s.append("version: " + mi->version.to_string() + "\n");
    if (!mi->dependencies.empty()) {
        s.append("dependencies:\n");
        for (auto dep : mi->dependencies)
            s.append("   " + dep.name + dep.op + dep.version.to_string() + "\n");
    }
    return s;
}

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

    auto description_panel = PackedEntity::spawn("core:ui:panel", r);
    if (auto* n = r.try_get<Node>(description_panel)) {
        n->width = Val::Percent(50.f);
        n->height = Val::Percent(100.f);
        n->padding = UiRect::all(Val::Px(20.f));
    }
    if (auto* fs = r.try_get<FlexStyle>(description_panel)) {
        fs->direction = FlexDirection::Column;
        fs->justify = JustifyContent::FlexStart;
        fs->align_items = AlignItems::FlexStart;
        fs->row_gap = 8.f;
    }
    hierarchy::set_parent(r, description_panel, root);

    auto mi_panel_mod_info = PackedEntity::spawn("core:ui:label", r);
    hierarchy::set_parent(r, mi_panel_mod_info, description_panel);


    auto title = PackedEntity::spawn("core:ui:title", r);
    if (auto* l = r.try_get<Label>(title)) {
        l->set_string("Mods");
    }
    hierarchy::set_parent(r, title, panel);

    for (const auto& mi : ModLoader::list()) {
        auto row = PackedEntity::spawn("core:ui:button", r);
        if (row == entt::null) continue;

        auto mi_name = mi.name;

        hierarchy::set_button_text(r, row, mi.name + " " + mi.version.to_string());
        hierarchy::set_parent(r, row, panel);

        if (auto* n = r.try_get<Node>(row)) {
            n->width = Val::Percent(100.f);
            n->height = Val::Px(44.f);
        }

        r.emplace<OnEvent>(row).cb =
            [
                mi_name,
                mi_panel_mod_info
            ](entt::registry& r, entt::entity, Event& ev) {
                if (ev.type == EventType::Click) {
                    selected_mod = mi_name;
                    if (auto* l = r.try_get<Label>(mi_panel_mod_info)) {
                        l->set_string(text_mod_info(selected_mod));
                    }
                        
                    ev.handled = true;
                }
            };
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

    auto reload_btn = PackedEntity::spawn("core:ui:button", r);
    hierarchy::set_button_text(r, reload_btn, "Reload");
    if (reload_btn != entt::null) {
        hierarchy::set_parent(r, reload_btn, panel);
        if (auto* n = r.try_get<Node>(reload_btn)) {
            n->width  = Val::Px(120.f);
            n->height = Val::Px(40.f);
        }
        if (auto* fs = r.try_get<FlexStyle>(reload_btn))
            fs->align_self = AlignSelf::Center;

        r.emplace<OnEvent>(reload_btn).cb =
            [mi_panel_mod_info](entt::registry& r, entt::entity, Event& ev) {
                if (ev.type == EventType::Click) {
                    ModLoader::load();
                    if (auto* l = r.try_get<Label>(mi_panel_mod_info)) {
                        l->set_string(text_mod_info(selected_mod));
                    }
                    ev.handled = true;
                }
            };
    }
}

void ModList::handle_event(sf::RenderWindow&, entt::registry&, const sf::Event&) {}
void ModList::on_exit(sf::RenderWindow&, entt::registry&) {}

}

ME_REGISTER_GAME_STATE("godlike:mod_list", godlike::states::ModList)