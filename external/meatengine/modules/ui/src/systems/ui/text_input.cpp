#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <meatengine/app/ctx/InputState.hpp>

#include <meatengine/ui/cmp/TextEditState.hpp>
#include <meatengine/ui/cmp/Node.hpp>
#include <meatengine/ui/cmp/States.hpp>
#include <meatengine/ui/cmp/ComputedNode.hpp>
#include <meatengine/spatial/cmp/GlobalBounds.hpp>

#include <SFML/Window/Clipboard.hpp>
#include <SFML/Graphics/Text.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

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
std::size_t pos_from_line_col(const std::u32string& s, std::size_t line_start_pos, std::size_t col) {
    std::size_t end = line_end(s, line_start_pos);
    return std::min(line_start_pos + col, end);
}
std::size_t column_of(const std::u32string& s, std::size_t pos) {
    return pos - line_start(s, pos);
}

void move_vertical(me::ui::cmp::TextEditState& st, int dir, bool select) {
    if (!st.multiline) {
        if (dir < 0) st.move_left(select, false); else st.move_right(select, false);
        return;
    }
    std::size_t ls = line_start(st.value, st.cursor);
    std::size_t col = st.preferred_column;
    if (dir < 0) {
        if (ls == 0) {
            st.cursor = 0;
            if (!select) st.anchor = st.cursor;
            st.preferred_column = 0;
            return;
        }
        std::size_t prev_end = ls - 1;
        std::size_t prev_start = line_start(st.value, prev_end);
        st.cursor = pos_from_line_col(st.value, prev_start, col);
    } else {
        std::size_t le = line_end(st.value, st.cursor);
        if (le >= st.value.size()) {
            st.cursor = st.value.size();
            if (!select) st.anchor = st.cursor;
            st.preferred_column = column_of(st.value, st.cursor);
            return;
        }
        std::size_t next_start = le + 1;
        st.cursor = pos_from_line_col(st.value, next_start, col);
    }
    if (!select) st.anchor = st.cursor;
}

sf::String from_u32(const std::u32string& s) { return sf::String(s); }

static float resolve_pad(const me::ui::cmp::Val& v, float parent) {
    switch (v.kind) {
        case me::ui::cmp::Val::Kind::Px: return v.value;
        case me::ui::cmp::Val::Kind::Percent: return parent * v.value / 100.f;
        default: return 0.f;
    }
}

static void sync_text_for_hit(me::ui::cmp::TextEditState& st, bool focused) {
    if (!st.text) return;
    sf::String shown;
    if (st.password && !st.value.empty())
        shown = sf::String(std::u32string(st.value.size(), st.password_char));
    else
        shown = sf::String(st.value);
    if (shown.isEmpty() && !st.placeholder.empty() && !focused) {
        st.text->setString(sf::String(st.placeholder));
    } else {
        st.text->setString(shown);
    }
    st.text->setCharacterSize(st.character_size);
}

static std::size_t index_from_mouse(entt::registry& r, entt::entity e, sf::Vector2f mouse_world) {
    auto* st = r.try_get<me::ui::cmp::TextEditState>(e);
    auto* gb = r.try_get<me::spatial::cmp::GlobalBounds>(e);
    if (!st || !st->text || !gb) return 0;

    float pad_l = 4.f, pad_t = 4.f;
    if (auto* n = r.try_get<me::ui::cmp::Node>(e)) {
        auto* cn = r.try_get<me::ui::cmp::ComputedNode>(e);
        float pw = cn ? cn->size.x : gb->rect.size.x;
        float ph = cn ? cn->size.y : gb->rect.size.y;
        pad_l = resolve_pad(n->padding.left, pw);
        pad_t = resolve_pad(n->padding.top, ph);
    }

    bool focused = r.any_of<me::ui::cmp::Focused>(e);
    sync_text_for_hit(*st, focused);
    st->text->setPosition({pad_l - st->scroll_x, pad_t - st->scroll_y});

    const float mx = mouse_world.x - gb->rect.position.x;
    const float my = mouse_world.y - gb->rect.position.y;

    const std::size_t count = st->text->getString().getSize();
    const float line_h = std::max(st->text->getFont().getLineSpacing(st->character_size),
                                  static_cast<float>(st->character_size) * 1.2f);

    auto index_in_line = [&](std::size_t lstart, std::size_t lend) -> std::size_t {
        if (lstart > lend) lstart = lend;
        std::size_t best_i = lstart;
        float best_dx = std::abs(st->text->findCharacterPos(std::min(lstart, count)).x - mx);
        for (std::size_t i = lstart + 1; i <= lend; ++i) {
            float x = st->text->findCharacterPos(std::min(i, count)).x;
            float dx = std::abs(x - mx);
            if (dx < best_dx) { best_dx = dx; best_i = i; }
        }
        if (best_i < lend) {
            float x0 = st->text->findCharacterPos(best_i).x;
            float x1 = st->text->findCharacterPos(best_i + 1).x;
            if (mx > (x0 + x1) * 0.5f) best_i += 1;
        } else if (best_i > lstart) {
            float x0 = st->text->findCharacterPos(best_i - 1).x;
            float x1 = st->text->findCharacterPos(best_i).x;
            if (mx < (x0 + x1) * 0.5f) best_i -= 1;
        }
        return std::min(best_i, st->value.size());
    };

    std::size_t best = 0;
    if (!st->multiline || st->value.empty()) {
        best = index_in_line(0, st->value.size());
    } else {
        struct LineInfo { std::size_t start; std::size_t end; float baseline_y; };
        std::vector<LineInfo> lines;
        std::size_t lstart = 0;
        while (true) {
            std::size_t nl = st->value.find(U'\n', lstart);
            std::size_t lend = (nl == std::u32string::npos) ? st->value.size() : nl;
            float by = st->text->findCharacterPos(lstart).y;
            lines.push_back({lstart, lend, by});
            if (nl == std::u32string::npos) break;
            lstart = nl + 1;
        }

        std::size_t chosen = 0;
        if (lines.size() == 1) {
            chosen = 0;
        } else {
            if (my < (lines[0].baseline_y + lines[1].baseline_y) * 0.5f - line_h * 0.5f) {
                chosen = 0;
            } else {
                chosen = lines.size() - 1;
                for (std::size_t li = 0; li + 1 < lines.size(); ++li) {
                    float mid = (lines[li].baseline_y + lines[li + 1].baseline_y) * 0.5f;
                    if (my < mid) { chosen = li; break; }
                }
            }
            float best_dy = std::numeric_limits<float>::max();
            for (std::size_t li = 0; li < lines.size(); ++li) {
                float dy = std::abs(my - (lines[li].baseline_y - line_h * 0.25f));
                if (dy < best_dy) { best_dy = dy; chosen = li; }
            }
        }

        best = index_in_line(lines[chosen].start, lines[chosen].end);
    }

    if (st->text->getString() == sf::String(st->placeholder) && st->value.empty() && !focused) {
        return 0;
    }
    return std::min<std::size_t>(best, st->value.size());
}

void run() {
    auto& r = me::MainLoop::get_registry();
    auto& in = r.ctx().get<me::app::ctx::InputState>();

    static entt::entity active_drag = entt::null;

    auto view = r.view<me::ui::cmp::TextEditState>();
    for (auto e : view) {
        auto& st = view.get<me::ui::cmp::TextEditState>(e);
        st.cursor_blink += me::MainLoop::get_dt();
        if (st.cursor_blink >= 0.5f) { st.cursor_blink = 0.f; st.cursor_visible = !st.cursor_visible; }
    }

    if (in.mouse_just_pressed) {
        entt::entity target = entt::null;
        for (auto e : view) {
            if (r.any_of<me::ui::cmp::Focused>(e) && r.any_of<me::ui::cmp::Hovered>(e)) { target = e; break; }
        }
        if (target != entt::null && r.valid(target)) {
            auto& st = view.get<me::ui::cmp::TextEditState>(target);
            std::size_t idx = index_from_mouse(r, target, in.mouse_pos);
            st.cursor = idx;
            if (!in.shift_down) st.anchor = idx;
            st.cursor_visible = true; st.cursor_blink = 0.f;
            active_drag = target;
            st.dirty = true;
        } else {
            active_drag = entt::null;
        }
    }

    if (in.mouse_down && active_drag != entt::null && r.valid(active_drag)) {
        if (auto* st = r.try_get<me::ui::cmp::TextEditState>(active_drag)) {
            if (r.any_of<me::ui::cmp::Focused>(active_drag)) {
                std::size_t idx = index_from_mouse(r, active_drag, in.mouse_pos);
                st->cursor = idx;
                st->cursor_visible = true; st->cursor_blink = 0.f;
                st->dirty = true;
            }
        } else {
            active_drag = entt::null;
        }
    } else if (!in.mouse_down) {
        active_drag = entt::null;
    }

    for (auto e : view) {
        auto& st = view.get<me::ui::cmp::TextEditState>(e);
        bool focused = r.any_of<me::ui::cmp::Focused>(e);
        if (!focused) continue;

        if (in.select_all_pressed) { st.select_all(); st.dirty = true; }

        if (in.copy_pressed || in.cut_pressed) {
            if (st.has_selection()) {
                sf::String s(st.selected_text());
                sf::Clipboard::setString(s);
                if (in.cut_pressed) st.delete_selection();
            }
        }
        if (in.paste_pressed) {
            auto sclip = sf::Clipboard::getString().toUtf32();
            if (!st.multiline) {
                std::u32string filtered;
                filtered.reserve(sclip.size());
                for (auto c : sclip) if (c != U'\n' && c != U'\r') filtered.push_back(c);
                sclip.swap(filtered);
            }
            if (!sclip.empty()) { st.insert_text(sclip); st.preferred_column = column_of(st.value, st.cursor); }
        }

        if (in.backspace_pressed) { st.backspace(); st.preferred_column = column_of(st.value, st.cursor); }
        if (in.delete_pressed) { st.del(); st.preferred_column = column_of(st.value, st.cursor); }

        if (in.left_pressed) {
            if (!in.shift_down && st.has_selection()) {
                st.cursor = st.sel_min();
                st.anchor = st.cursor;
            } else {
                st.move_left(in.shift_down, in.ctrl_down);
            }
            st.cursor_visible = true; st.cursor_blink = 0.f;
        }
        if (in.right_pressed) {
            if (!in.shift_down && st.has_selection()) {
                st.cursor = st.sel_max();
                st.anchor = st.cursor;
            } else {
                st.move_right(in.shift_down, in.ctrl_down);
            }
            st.cursor_visible = true; st.cursor_blink = 0.f;
        }
        if (in.up_pressed) {
            if (!in.shift_down && st.has_selection()) {
                st.cursor = st.sel_min();
                st.anchor = st.cursor;
            }
            move_vertical(st, -1, in.shift_down);
            st.cursor_visible = true; st.cursor_blink = 0.f;
        }
        if (in.down_pressed) {
            if (!in.shift_down && st.has_selection()) {
                st.cursor = st.sel_max();
                st.anchor = st.cursor;
            }
            move_vertical(st, +1, in.shift_down);
            st.cursor_visible = true; st.cursor_blink = 0.f;
        }

        if (in.home_pressed) {
            if (!in.shift_down && st.has_selection()) {
                st.cursor = st.sel_min();
                st.anchor = st.cursor;
            }
            if (st.multiline) st.cursor = line_start(st.value, st.cursor); else st.cursor = 0;
            if (!in.shift_down) st.anchor = st.cursor;
        }
        if (in.end_pressed) {
            if (!in.shift_down && st.has_selection()) {
                st.cursor = st.sel_max();
                st.anchor = st.cursor;
            }
            if (st.multiline) st.cursor = line_end(st.value, st.cursor); else st.cursor = st.value.size();
            if (!in.shift_down) st.anchor = st.cursor;
        }

        if (in.enter_pressed) {
            if (st.multiline) { st.insert_text(U"\n"); st.preferred_column = column_of(st.value, st.cursor); }
        }

        if (!in.text_entered.empty()) {
            std::u32string filtered;
            filtered.reserve(in.text_entered.size());
            for (auto c : in.text_entered) {
                if (!st.multiline && (c == U'\n' || c == U'\r')) continue;
                if (c == U'\t') { filtered.push_back(U' '); filtered.push_back(U' '); continue; }
                if (is_printable(c)) filtered.push_back(c);
            }
            if (!filtered.empty()) { st.insert_text(filtered); st.preferred_column = column_of(st.value, st.cursor); }
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

