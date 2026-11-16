#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <meatengine/app/ctx/InputState.hpp>

#include <meatengine/ui/cmp/TextEditState.hpp>
#include <meatengine/ui/cmp/States.hpp>
#include <meatengine/ui/cmp/ComputedNode.hpp>
#include <meatengine/spatial/cmp/GlobalTransform.hpp>
#include <meatengine/spatial/cmp/GlobalBounds.hpp>

#include <SFML/Window/Clipboard.hpp>
#include <SFML/Graphics/Text.hpp>

#include <algorithm>
#include <cmath>

namespace {

using namespace me;

bool is_printable(char32_t c) {
    return c >= 0x20 && c != 0x7F;
}

std::size_t line_start(const std::u32string& s, std::size_t pos) {
    auto p = s.rfind(U'\n', pos == 0 ? 0 : pos - 1);
    return (p == std::u32string::npos) ? 0 : p + 1;
}
std::size_t line_end(const std::u32string& s, std::size_t pos) {
    auto p = s.find(U'\n', pos);
    return (p == std::u32string::npos) ? s.size() : p;
}
std::size_t column_at(const std::u32string& s, std::size_t pos) {
    return pos - line_start(s, pos);
}
std::size_t pos_from_line_col(const std::u32string& s, std::size_t line_start_pos, std::size_t col) {
    std::size_t end = line_end(s, line_start_pos);
    return std::min(line_start_pos + col, end);
}

void move_vertical(me::ui::cmp::TextEditState& st, int dir, bool select) {
    if (!st.multiline) {
        if (dir < 0) st.move_left(select, false); else st.move_right(select, false);
        return;
    }
    std::size_t ls = line_start(st.value, st.cursor);
    std::size_t col = st.cursor - ls;
    if (dir < 0) {
        if (ls == 0) { st.cursor = 0; if (!select) st.anchor = st.cursor; return; }
        std::size_t prev_end = ls - 1;
        std::size_t prev_start = line_start(st.value, prev_end);
        st.cursor = pos_from_line_col(st.value, prev_start, col);
    } else {
        std::size_t le = line_end(st.value, st.cursor);
        if (le >= st.value.size()) { st.cursor = st.value.size(); if (!select) st.anchor = st.cursor; return; }
        std::size_t next_start = le + 1;
        st.cursor = pos_from_line_col(st.value, next_start, col);
    }
    if (!select) st.anchor = st.cursor;
}

std::u32string to_u32(const sf::String& s) { return s.toUtf32(); }
sf::String from_u32(const std::u32string& s) { return sf::String(s); }

void run() {
    auto& r = me::MainLoop::get_registry();
    auto& in = r.ctx().get<me::app::ctx::InputState>();

    auto view = r.view<me::ui::cmp::TextEditState>();
    for (auto e : view) {
        auto& st = view.get<me::ui::cmp::TextEditState>(e);
        bool focused = r.any_of<me::ui::cmp::Focused>(e);
        st.cursor_blink += me::MainLoop::get_dt();
        if (st.cursor_blink >= 0.5f) { st.cursor_blink = 0.f; st.cursor_visible = !st.cursor_visible; }

        if (!focused) continue;

        // click to place caret (approximate by local x) when just pressed & hovered
        if (in.mouse_just_pressed && r.any_of<me::ui::cmp::Hovered>(e)) {
            if (auto* gb = r.try_get<me::spatial::cmp::GlobalBounds>(e)) {
                float local_x = in.mouse_pos.x - gb->rect.position.x - 6.f + st.scroll_x;
                if (st.text) {
                    // find closest character index
                    sf::String shown = st.password && !st.value.empty()
                        ? sf::String(std::u32string(st.value.size(), st.password_char))
                        : from_u32(st.value);
                    st.text->setString(shown);
                    std::size_t best = 0;
                    float best_d = 1e9f;
                    for (std::size_t i = 0; i <= shown.getSize(); ++i) {
                        auto pos = st.text->findCharacterPos(i);
                        float d = std::abs(pos.x - local_x);
                        if (d < best_d) { best_d = d; best = i; }
                    }
                    st.cursor = std::min<std::size_t>(best, st.value.size());
                    if (!in.shift_down) st.anchor = st.cursor;
                    st.cursor_visible = true; st.cursor_blink = 0.f;
                    st.dirty = true;
                }
            }
        }

        if (in.select_all_pressed) { st.select_all(); st.dirty = true; }

        if (in.copy_pressed || in.cut_pressed) {
            if (st.has_selection()) {
                sf::String s(st.selected_text());
                sf::Clipboard::setString(s);
                if (in.cut_pressed) st.delete_selection();
            }
        }
        if (in.paste_pressed) {
            auto s = sf::Clipboard::getString().toUtf32();
            if (!st.multiline) {
                // strip newlines for single-line
                std::u32string filtered;
                filtered.reserve(s.size());
                for (auto c : s) if (c != U'\n' && c != U'\r') filtered.push_back(c);
                s.swap(filtered);
            }
            st.insert_text(s);
        }

        if (in.backspace_pressed) st.backspace();
        if (in.delete_pressed) st.del();

        if (in.left_pressed)  { st.move_left(in.shift_down, in.ctrl_down);  st.cursor_visible = true; st.cursor_blink = 0.f; }
        if (in.right_pressed) { st.move_right(in.shift_down, in.ctrl_down); st.cursor_visible = true; st.cursor_blink = 0.f; }
        if (in.up_pressed)    { move_vertical(st, -1, in.shift_down); st.cursor_visible = true; st.cursor_blink = 0.f; }
        if (in.down_pressed)  { move_vertical(st, +1, in.shift_down); st.cursor_visible = true; st.cursor_blink = 0.f; }

        if (in.home_pressed) {
            if (st.multiline) st.cursor = line_start(st.value, st.cursor); else st.cursor = 0;
            if (!in.shift_down) st.anchor = st.cursor;
        }
        if (in.end_pressed) {
            if (st.multiline) st.cursor = line_end(st.value, st.cursor); else st.cursor = st.value.size();
            if (!in.shift_down) st.anchor = st.cursor;
        }

        if (in.enter_pressed) {
            if (st.multiline) st.insert_text(U"\n");
            // LineEdit: Enter could emit submit event in future; ignore for now
        }

        if (!in.text_entered.empty()) {
            std::u32string filtered;
            filtered.reserve(in.text_entered.size());
            for (auto c : in.text_entered) {
                if (!st.multiline && (c == U'\n' || c == U'\r')) continue;
                if (c == U'\t') { filtered.push_back(U' '); filtered.push_back(U' '); continue; }
                if (is_printable(c)) filtered.push_back(c);
            }
            if (!filtered.empty()) st.insert_text(filtered);
        }

        st.clamp();
    }
}

}

ME_REGISTER_SYSTEM("engine:ui:text_input",
    me::SystemPhase::Input,
    12,
    run
)
