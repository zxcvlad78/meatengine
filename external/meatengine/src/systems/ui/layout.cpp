#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/Transform.hpp>
#include <meatengine/cmp/Children.hpp>
#include <meatengine/cmp/Parent.hpp>

#include <meatengine/cmp/ui/Node.hpp>
#include <meatengine/cmp/ui/FlexStyle.hpp>
#include <meatengine/cmp/ui/ComputedNode.hpp>
#include <meatengine/cmp/ui/FillRect.hpp>
#include <meatengine/cmp/ui/Label.hpp>

#include <algorithm>
#include <vector>

namespace {

using namespace me;
using namespace me::cmp::ui;

float resolve_val(const Val& v, float parent_size) {
    switch (v.kind) {
        case Val::Kind::Px: return v.value;
        case Val::Kind::Percent: return parent_size * v.value / 100.f;
        default: return 0.f;
    }
}

sf::Vector2f intrinsic_size(entt::registry& r, entt::entity e) {
    sf::Vector2f s{0.f, 0.f};

    if (auto* fr = r.try_get<me::cmp::ui::FillRect>(e))
        s = fr->shape.getSize();

    if (auto* lb = r.try_get<me::cmp::ui::Label>(e); lb && lb->text) {
        auto b = lb->text->getLocalBounds();
        s.x = std::max(s.x, b.size.x);
        s.y = std::max(s.y, b.size.y);
    }

    if (auto* ch = r.try_get<me::cmp::Children>(e); ch && !ch->entities.empty()) {
        if (auto* fs = r.try_get<FlexStyle>(e);
            fs && fs->display == Display::Flex)
        {
            const bool row = (fs->direction == FlexDirection::Row
                || fs->direction == FlexDirection::RowReverse);
            float main = 0.f, cross = 0.f;
            bool first = true;
            for (auto c : ch->entities) {
                if (!r.valid(c)) continue;
                auto cs = intrinsic_size(r, c);
                if (row) {
                    main += cs.x;
                    if (!first) main += fs->column_gap;
                    cross = std::max(cross, cs.y);
                } else {
                    main += cs.y;
                    if (!first) main += fs->row_gap;
                    cross = std::max(cross, cs.x);
                }
                first = false;
            }
            if (row) { s.x = std::max(s.x, main); s.y = std::max(s.y, cross); }
            else { s.y = std::max(s.y, main); s.x = std::max(s.x, cross); }
        }
    }

    if (auto* n = r.try_get<Node>(e)) {
        s.x += resolve_val(n->padding.left, s.x) + resolve_val(n->padding.right, s.x);
        s.y += resolve_val(n->padding.top,  s.y) + resolve_val(n->padding.bottom, s.y);
    }
    return s;
}

void layout_children(
    entt::registry& r,
    entt::entity parent,
    const sf::FloatRect& parent_local_rect
);

void layout_absolute_item(
    entt::registry& r,
    entt::entity e,
    const sf::FloatRect& content_box
) {
    auto* n = r.try_get<Node>(e);
    auto& cn = r.get_or_emplace<ComputedNode>(e);

    auto is = intrinsic_size(r, e);

    float w = (n && !n->width.is_auto()) ? resolve_val(n->width, content_box.size.x)
        : is.x;
    float h = (n && !n->height.is_auto()) ? resolve_val(n->height, content_box.size.y)
        : is.y;

    float x = 0.f, y = 0.f;
    if (n) {
        if (!n->left.is_auto())   x = resolve_val(n->left, content_box.size.x);
        if (!n->top.is_auto()) y = resolve_val(n->top, content_box.size.y);
        if (!n->right.is_auto())
            x = content_box.size.x - w - resolve_val(n->right, content_box.size.x);
        if (!n->bottom.is_auto())
            y = content_box.size.y - h - resolve_val(n->bottom, content_box.size.y);
    }

    cn.size = {w, h};

    auto* t = r.try_get<me::cmp::Transform>(e);
    if (!t) t = &r.emplace<me::cmp::Transform>(e);
    t->position = content_box.position + sf::Vector2f{x, y};

    if (auto* fr = r.try_get<me::cmp::ui::FillRect>(e)) {
        if (fr->shape.getSize() != cn.size) {
            fr->shape.setSize(cn.size);
            fr->dirty = true;
        }
    }

    layout_children(r, e, sf::FloatRect{sf::Vector2f{0.f, 0.f}, cn.size});
}

void layout_axis(
    entt::registry& r,
    entt::entity parent,
    const FlexStyle& fs,
    const sf::FloatRect& content_box
) {
    auto* ch = r.try_get<me::cmp::Children>(parent);
    if (!ch || ch->entities.empty()) return;

    std::vector<entt::entity> flex_items;
    std::vector<entt::entity> abs_items;
    for (auto e : ch->entities) {
        if (!r.valid(e)) continue;
        auto* n = r.try_get<Node>(e);
        if (n && n->position == Node::PositionType::Absolute)
            abs_items.push_back(e);
        else
            flex_items.push_back(e);
    }

    for (auto e : abs_items)
        layout_absolute_item(r, e, content_box);

    if (flex_items.empty()) return;

    const bool row = (fs.direction == FlexDirection::Row
        || fs.direction == FlexDirection::RowReverse);
    const bool reverse = (fs.direction == FlexDirection::RowReverse
        || fs.direction == FlexDirection::ColumnReverse);

    const float main_avail = row ? content_box.size.x : content_box.size.y;
    const float cross_avail = row ? content_box.size.y : content_box.size.x;
    const float main_gap = row ? fs.column_gap : fs.row_gap;

    std::vector<float> main_sizes(flex_items.size());
    std::vector<float> cross_sizes(flex_items.size());
    float total_main = 0.f, total_grow = 0.f;

    for (size_t i = 0; i < flex_items.size(); ++i) {
        auto e = flex_items[i];
        auto* n = r.try_get<Node>(e);
        auto& cn = r.get_or_emplace<ComputedNode>(e);

        if (cn.size.x == 0.f && cn.size.y == 0.f) {
            auto is = intrinsic_size(r, e);
            bool need_w = (!n || n->width.is_auto())  && cn.size.x == 0.f;
            bool need_h = (!n || n->height.is_auto()) && cn.size.y == 0.f;
            if (need_w) cn.size.x = is.x;
            if (need_h) cn.size.y = is.y;
        }

        float m = row ? cn.size.x : cn.size.y;
        float c = row ? cn.size.y : cn.size.x;

        if (!fs.flex_basis.is_auto())
            m = resolve_val(fs.flex_basis, main_avail);
        else if (n) {
            auto& sz = row ? n->width : n->height;
            if (!sz.is_auto()) m = resolve_val(sz, main_avail);
        }
        if (n) {
            auto& sz = row ? n->height : n->width;
            if (!sz.is_auto()) c = resolve_val(sz, cross_avail);
        }

        main_sizes[i] = m;
        cross_sizes[i] = c;
        total_main += m;

        if (auto* cfs = r.try_get<FlexStyle>(e))
            total_grow += cfs->flex_grow;
    }
    total_main += main_gap * (flex_items.size() > 1 ? float(flex_items.size() - 1) : 0.f);

    float free = main_avail - total_main;
    if (free > 0.f && total_grow > 0.f) {
        for (size_t i = 0; i < flex_items.size(); ++i)
            if (auto* cfs = r.try_get<FlexStyle>(flex_items[i]))
                if (cfs->flex_grow > 0.f)
                    main_sizes[i] += free * (cfs->flex_grow / total_grow);
    } else if (free < 0.f) {
        float total_shrink = 0.f;
        for (auto e : flex_items)
            if (auto* cfs = r.try_get<FlexStyle>(e))
                total_shrink += cfs->flex_shrink;
        if (total_shrink > 0.f)
            for (size_t i = 0; i < flex_items.size(); ++i)
                if (auto* cfs = r.try_get<FlexStyle>(flex_items[i]))
                    if (cfs->flex_shrink > 0.f)
                        main_sizes[i] += free * (cfs->flex_shrink / total_shrink);
    }

    float used = 0.f;
    for (auto m : main_sizes) used += m;
    used += main_gap * (flex_items.size() > 1 ? float(flex_items.size() - 1) : 0.f);
    float leftover = main_avail - used;

    float cursor = 0.f;
    float gap = main_gap;
    switch (fs.justify) {
        case JustifyContent::FlexStart: cursor = 0.f; break;
        case JustifyContent::FlexEnd: cursor = leftover; break;
        case JustifyContent::Center: cursor = leftover * 0.5f; break;
        case JustifyContent::SpaceBetween:
            gap = flex_items.size() > 1
                ? main_gap + leftover / float(flex_items.size() - 1) : 0.f;
            break;
        case JustifyContent::SpaceAround:
            gap = main_gap + leftover / float(flex_items.size());
            cursor = gap * 0.5f;
            break;
        case JustifyContent::SpaceEvenly:
            gap = main_gap + leftover / float(flex_items.size() + 1);
            cursor = gap;
            break;
    }

    for (size_t idx = 0; idx < flex_items.size(); ++idx) {
        size_t i = reverse ? flex_items.size() - 1 - idx : idx;
        auto e = flex_items[i];

        auto* n = r.try_get<Node>(e);
        auto* cfs = r.try_get<FlexStyle>(e);
        auto& cn = r.get_or_emplace<ComputedNode>(e);

        float main_pos = reverse ? main_avail - cursor - main_sizes[i] : cursor;
        float cross_size = cross_sizes[i];

        AlignItems al = fs.align_items;
        if (cfs && cfs->align_self != AlignSelf::Auto) {
            switch (cfs->align_self) {
                case AlignSelf::FlexStart: al = AlignItems::FlexStart; break;
                case AlignSelf::FlexEnd: al = AlignItems::FlexEnd; break;
                case AlignSelf::Center: al = AlignItems::Center; break;
                case AlignSelf::Stretch: al = AlignItems::Stretch; break;
                default: break;
            }
        }
        if (al == AlignItems::Stretch
            && n && (row ? n->height.is_auto() : n->width.is_auto()))
            cross_size = cross_avail;

        float cross_pos = 0.f;
        switch (al) {
            case AlignItems::FlexStart: cross_pos = 0.f; break;
            case AlignItems::FlexEnd: cross_pos = cross_avail - cross_size; break;
            case AlignItems::Center: cross_pos = (cross_avail - cross_size) * 0.5f; break;
            default: cross_pos = 0.f; break;
        }

        sf::Vector2f local = row
            ? sf::Vector2f{main_pos, cross_pos}
            : sf::Vector2f{cross_pos, main_pos};

        float final_w = row ? main_sizes[i] : cross_size;
        float final_h = row ? cross_size : main_sizes[i];

        auto* t = r.try_get<me::cmp::Transform>(e);
        if (!t) t = &r.emplace<me::cmp::Transform>(e);
        t->position = content_box.position + local;

        cn.size = {final_w, final_h};

        if (auto* fr = r.try_get<me::cmp::ui::FillRect>(e)) {
            if (fr->shape.getSize() != cn.size) {
                fr->shape.setSize(cn.size);
                fr->dirty = true;
            }
        }

        layout_children(r, e, sf::FloatRect{sf::Vector2f{0.f, 0.f}, cn.size});

        cursor += main_sizes[i] + gap;
    }
}

void layout_children(
    entt::registry& r, 
    entt::entity parent,
    const sf::FloatRect& parent_local_rect
) {
    auto* fs = r.try_get<FlexStyle>(parent);
    auto* n  = r.try_get<Node>(parent);

    sf::FloatRect content = parent_local_rect;
    if (n) {
        content.position.x += resolve_val(n->padding.left, parent_local_rect.size.x);
        content.position.y += resolve_val(n->padding.top,  parent_local_rect.size.y);
        content.size.x -= resolve_val(n->padding.left, parent_local_rect.size.x)
                            + resolve_val(n->padding.right, parent_local_rect.size.x);
        content.size.y -= resolve_val(n->padding.top,  parent_local_rect.size.y)
                            + resolve_val(n->padding.bottom, parent_local_rect.size.y);
        if (content.size.x < 0.f) content.size.x = 0.f;
        if (content.size.y < 0.f) content.size.y = 0.f;
    }

    if (!fs || fs->display == Display::None) {
        if (auto* ch = r.try_get<me::cmp::Children>(parent))
            for (auto c : ch->entities)
                if (r.valid(c))
                    layout_children(r, c, sf::FloatRect{{0.f, 0.f}, {0.f, 0.f}});
        return;
    }
    if (fs->display == Display::Grid) return;

    layout_axis(r, parent, *fs, content);
}

void run() {
    auto& r = me::MainLoop::get_registry();
    auto& win = me::MainLoop::get_window();
    sf::FloatRect viewport{{0.f, 0.f}, static_cast<sf::Vector2f>(win.getSize())};

    for (auto [e, n] : r.view<Node>(entt::exclude<me::cmp::Parent>).each()) {
        auto& cn = r.get_or_emplace<ComputedNode>(e);

        float w = n.width.is_auto()  ? viewport.size.x : resolve_val(n.width,  viewport.size.x);
        float h = n.height.is_auto() ? viewport.size.y : resolve_val(n.height, viewport.size.y);
        cn.size = {w, h};

        auto* t = r.try_get<me::cmp::Transform>(e);
        if (!t) t = &r.emplace<me::cmp::Transform>(e);
        t->position = viewport.position;

        layout_children(r, e, sf::FloatRect{sf::Vector2f{0.f, 0.f}, cn.size});
    }
}

}

ME_REGISTER_SYSTEM("engine:ui:layout",
    me::SystemPhase::Update,
    2,
    run
)