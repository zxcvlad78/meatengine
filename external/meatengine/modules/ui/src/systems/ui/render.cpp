#include <meatengine/ecs/SystemRegistry.hpp>
#include <meatengine/app/MainLoop.hpp>

#include <meatengine/ui/cmp/ComputedNode.hpp>
#include <meatengine/spatial/cmp/GlobalTransform.hpp>
#include <meatengine/ui/cmp/FillRect.hpp>
#include <meatengine/ui/cmp/Label.hpp>
#include <meatengine/ui/cmp/TextEditState.hpp>
#include <meatengine/ui/cmp/Node.hpp>
#include <meatengine/ui/cmp/States.hpp>
#include <meatengine/hierarchy/cmp/Parent.hpp>
#include <meatengine/graphics/cmp/Visibility.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <algorithm>
#include <vector>

namespace {

using namespace me;

static bool is_visible(entt::registry& r, entt::entity e) {
    entt::entity cur = e;
    while (cur != entt::null) {
        if (auto* vis = r.try_get<me::graphics::cmp::Visibility>(cur)) {
            if (!vis->value) return false;
        }
        if (auto* p = r.try_get<me::hierarchy::cmp::Parent>(cur)) {
            if (p->entity == entt::null) break;
            cur = p->entity;
        } else {
            break;
        }
    }
    return true;
}

static float resolve_pad(const me::ui::cmp::Val& v, float parent) {
    switch (v.kind) {
        case me::ui::cmp::Val::Kind::Px: return v.value;
        case me::ui::cmp::Val::Kind::Percent: return parent * v.value / 100.f;
        default: return 0.f;
    }
}

void run() {
    auto& r = me::MainLoop::get_registry();
    auto& win = me::MainLoop::get_window();

    std::vector<entt::entity> order;
    for (auto [e, _] : r.view<me::ui::cmp::ComputedNode>().each())
        order.push_back(e);

    std::sort(order.begin(), order.end(), [&](entt::entity a, entt::entity b) {
        return r.get<me::ui::cmp::ComputedNode>(a).stack_index
            < r.get<me::ui::cmp::ComputedNode>(b).stack_index;
    });

    for (auto e : order) {
        if (!is_visible(r, e)) continue;

        sf::Transform xf = r.try_get<me::spatial::cmp::GlobalTransform>(e)
                        ? r.get<me::spatial::cmp::GlobalTransform>(e).transform
                        : sf::Transform{};

        if (auto* fr = r.try_get<me::ui::cmp::FillRect>(e))
            win.draw(fr->shape, xf);

        if (auto* lb = r.try_get<me::ui::cmp::Label>(e); lb && lb->text)
            win.draw(*lb->text, xf);

        if (auto* st = r.try_get<me::ui::cmp::TextEditState>(e); st && st->text) {
            float pad_l = 4.f, pad_t = 4.f, pad_r = 4.f, pad_b = 4.f;
            if (auto* n = r.try_get<me::ui::cmp::Node>(e)) {
                auto* cn = r.try_get<me::ui::cmp::ComputedNode>(e);
                float pw = cn ? cn->size.x : 0.f;
                float ph = cn ? cn->size.y : 0.f;
                pad_l = resolve_pad(n->padding.left, pw);
                pad_t = resolve_pad(n->padding.top, ph);
                pad_r = resolve_pad(n->padding.right, pw);
                pad_b = resolve_pad(n->padding.bottom, ph);
            }

            sf::String shown;
            if (st->password && !st->value.empty())
                shown = sf::String(std::u32string(st->value.size(), st->password_char));
            else
                shown = sf::String(st->value);

            if (shown.isEmpty() && !st->placeholder.empty() && !r.any_of<me::ui::cmp::Focused>(e)) {
                st->text->setString(sf::String(st->placeholder));
                st->text->setFillColor(st->placeholder_color);
            } else {
                st->text->setString(shown);
                st->text->setFillColor(st->text_color);
            }
            st->text->setCharacterSize(st->character_size);
            st->text->setPosition({pad_l - st->scroll_x, pad_t - st->scroll_y});

            const auto shownSize = st->text->getString().getSize();
            const float line_h = st->text->getFont().getLineSpacing(st->character_size);
            const float caret_h = line_h;

            if (st->has_selection() && r.any_of<me::ui::cmp::Focused>(e)) {
                std::size_t a = std::min(st->sel_min(), (std::size_t)shownSize);
                std::size_t bsel = std::min(st->sel_max(), (std::size_t)shownSize);
                const auto& sref = st->value;

                std::size_t i = a;
                while (i < bsel) {
                    std::size_t line_break = i;
                    while (line_break < sref.size() && sref[line_break] != U'\n') ++line_break;

                    std::size_t seg_end = std::min(bsel, line_break);

                    sf::Vector2f p1 = st->text->findCharacterPos(std::min(i, (std::size_t)shownSize));

                    float right_x = p1.x;
                    if (seg_end > i) {
                        right_x = st->text->findCharacterPos(std::min(seg_end, (std::size_t)shownSize)).x;
                        if (right_x <= p1.x + 0.01f) {
                            for (std::size_t k = i + 1; k <= seg_end; ++k) {
                                float xk = st->text->findCharacterPos(std::min(k, (std::size_t)shownSize)).x;
                                if (xk > right_x) right_x = xk;
                            }
                        }
                    }

                    float w = right_x - p1.x;

                    if (bsel > line_break && line_break < sref.size() && sref[line_break] == U'\n') {
                        float nl_x = st->text->findCharacterPos(std::min(line_break, (std::size_t)shownSize)).x;
                        if (nl_x > right_x) w = std::max(w, nl_x - p1.x);
                        w += static_cast<float>(st->character_size) * 0.4f;
                    }

                    if (w < 0.5f && bsel > i) {
                        w = static_cast<float>(st->character_size) * 0.4f;
                    }

                    if (w > 0.f) {
                        st->selection_shape.setFillColor(st->selection_color);
                        st->selection_shape.setPosition({p1.x, p1.y});
                        st->selection_shape.setSize({w, line_h});
                        win.draw(st->selection_shape, xf);
                    }

                    if (line_break >= bsel) break;
                    i = line_break + 1;
                }
            }

            win.draw(*st->text, xf);

            if (r.any_of<me::ui::cmp::Focused>(e) && st->cursor_visible && !st->read_only) {
                auto cp = st->text->findCharacterPos(std::min(st->cursor, (std::size_t)shownSize));
                st->cursor_shape.setPosition({cp.x, cp.y});
                st->cursor_shape.setSize({1.f, caret_h});
                st->cursor_shape.setFillColor(st->cursor_color);
                win.draw(st->cursor_shape, xf);
            }
        }
    }
}

}

ME_REGISTER_SYSTEM("engine:ui:render",
    me::SystemPhase::PostRender,
    8,
    run
)
