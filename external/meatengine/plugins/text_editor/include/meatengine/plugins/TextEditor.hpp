#pragma once

#include <meatengine/app/Plugin.hpp>
#include <meatengine/app/App.hpp>
#include <meatengine/app/MainLoop.hpp>

#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/ecs/SystemPhase.hpp>

#include <meatengine/abstract/cmp/ZIndex.hpp>
#include <meatengine/graphics/cmp/Visibility.hpp>

#include <meatengine/ui/cmp/Node.hpp>
#include <meatengine/ui/cmp/FlexStyle.hpp>
#include <meatengine/ui/cmp/FillRect.hpp>
#include <meatengine/ui/cmp/BackgroundColor.hpp>
#include <meatengine/ui/cmp/Label.hpp>
#include <meatengine/ui/cmp/OnEvent.hpp>
#include <meatengine/ui/cmp/TextEditState.hpp>

#include <meatengine/ui/widgets/ui_root.hpp>
#include <meatengine/ui/widgets/panel.hpp>
#include <meatengine/ui/widgets/label.hpp>
#include <meatengine/ui/widgets/button.hpp>
#include <meatengine/ui/widgets/text_edit.hpp>

#include <meatengine/assets/FileSystem.hpp>

#include <entt/entt.hpp>
#include <SFML/Graphics/Color.hpp>

namespace {
    struct TextEditorTextEdit {};

    struct TextFile {
        std::string path;
        std::string text;
    };

    entt::entity make_tab(entt::registry& reg, entt::entity tabbar, const std::string& path) {
        auto tab = me::ui::widgets::make_button(reg, tabbar, path);
        reg.emplace<TextFile>(tab, path);
        if (auto* l = reg.try_get<me::ui::cmp::Label>(tab)) {
            l->set_character_size(8);
        }
        auto& ev = reg.emplace_or_replace<me::ui::cmp::OnEvent>(tab);
        ev.cb = [](entt::registry& r, entt::entity e, me::ui::cmp::Event& event) {
            if (!r.valid(e)){
                return;
            }
            auto* tf = r.try_get<TextFile>(e);
            if (!tf) {
                return;
            }

            if (event.type == me::ui::cmp::EventType::Click) {
                event.handled = true;

                std::string file_text;
                if (!me::FileSystem::get_file_text(tf->path, file_text)) return;
                tf->text = file_text;

                for (auto [text_edit_entity, text_edit_state] : r.view<me::ui::cmp::TextEditState, TextEditorTextEdit>().each()) {
                    text_edit_state.set_value_utf8(file_text);
                    text_edit_state.cursor = 0;
                    text_edit_state.anchor = 0;
                    text_edit_state.scroll_x = 0.f;
                    text_edit_state.scroll_y = 0.f;
                }

            }
        };
        
        return tab;
    }
}

namespace me {
class TextEditorPlugin : public app::Plugin {
public:
    TextFile* current_file;

    const char* name() const override { return "TextEditorPlugin"; }

    void build(App&) override {
        auto& reg = MainLoop::get_registry();
        using namespace me;
        using namespace me::ui;

        const sf::Color col_bg{30, 30, 30};
        const sf::Color col_tab{51, 51, 51};
        const sf::Color col_text{212, 212, 212};
        const sf::Color col_muted{160, 160, 160};

        auto ui_root = widgets::make_ui_root(reg);
        {
            reg.emplace_or_replace<me::abstract::cmp::ZIndex>(ui_root, 10);
            auto& node = reg.get<cmp::Node>(ui_root);
            node.width = cmp::Val::Percent(100.f);
            node.height = cmp::Val::Percent(100.f);
            auto& flex = reg.get<cmp::FlexStyle>(ui_root);
            flex.direction = cmp::FlexDirection::Column;
            flex.justify = cmp::JustifyContent::FlexStart;
            flex.align_items = cmp::AlignItems::Stretch;
            reg.get_or_emplace<cmp::BackgroundColor>(ui_root).color = col_bg;
            reg.get_or_emplace<cmp::FillRect>(ui_root).shape.setFillColor(col_bg);
            reg.get_or_emplace<graphics::cmp::Visibility>(ui_root).value = false;
        }

        auto tabbar = widgets::make_panel(reg, ui_root);
        {
            auto& node = reg.get<cmp::Node>(tabbar);
            node.width = cmp::Val::Percent(100.f);
            node.height = cmp::Val::Px(34.f);
            node.padding = cmp::UiRect::axes(cmp::Val::Px(8.f), cmp::Val::Px(12.f));
            auto& flex = reg.get<cmp::FlexStyle>(tabbar);
            flex.direction = cmp::FlexDirection::Row;
            flex.align_items = cmp::AlignItems::Center;
            reg.get_or_emplace<cmp::BackgroundColor>(tabbar).color = col_tab;
        }

        make_tab(reg, tabbar, "data/core/res/packed_entities.lua");

        auto text_edit = widgets::make_text_edit(reg, ui_root);
        {
            reg.emplace<TextEditorTextEdit>(text_edit);
            auto& node = reg.get<cmp::Node>(text_edit);
            node.width = cmp::Val::Percent(100.f);
            node.height = cmp::Val::Auto();
            node.padding = cmp::UiRect::all(cmp::Val::Px(12.f));
            auto& flex = reg.get<cmp::FlexStyle>(text_edit);
            flex.flex_grow = 1.f;
            flex.align_items = cmp::AlignItems::Stretch;
            reg.get_or_emplace<cmp::BackgroundColor>(text_edit).color = col_bg;

            if (auto* st = reg.try_get<cmp::TextEditState>(text_edit)) {
                st->set_value_utf8("");
                st->text_color = col_text;
                st->placeholder_color = col_muted;
                st->cursor_color = sf::Color::White;
                st->selection_color = sf::Color(38, 79, 120, 180);
                st->character_size = 16;
                if (st->text) st->text->setFillColor(col_text);
            }
            if (auto* fr = reg.try_get<cmp::FillRect>(text_edit)) {
                fr->shape.setFillColor(col_bg);
                fr->shape.setOutlineThickness(0.f);
            }
        }

        MainLoop::add_on_event([ui_root](const sf::Event& event, sf::RenderWindow&) {
            const auto* key = event.getIf<sf::Event::KeyPressed>();
            if (!key) return;
            auto& r = MainLoop::get_registry();
            if (key->code == sf::Keyboard::Key::F1 && key->control) {
                auto& visibility = r.get_or_emplace<graphics::cmp::Visibility>(ui_root);
                visibility.value = !visibility.value;
            }
        });

        //SystemRegistry::emplace(SystemPhase::Update, "engine:plugin:text_editor", 0, []() {});
    }
};
}
