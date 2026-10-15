#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>
#include <meatengine/SpriteBatcher.hpp>

#include <meatengine/cmp/ZIndex.hpp>
#include <meatengine/cmp/Sprite.hpp>

#include <algorithm>
#include <vector>

namespace {

using namespace me;

struct RenderEntry {
    sf::Sprite* sprite;
    int z;
};

struct BucketStore {
    std::vector<RenderEntry> flat;
    std::vector<int> zs_unique;
    std::unordered_map<int, std::pair<uint32_t, uint32_t>> z_range; // z -> [begin, end)

    void clear() {
        flat.clear();
        zs_unique.clear();
        z_range.clear();
    }
};

BucketStore& store() { static BucketStore s; return s; }

SpriteBatcher& batcher() { static SpriteBatcher b; return b; }

void sprite_render() {
    auto& reg = MainLoop::get_registry();
    auto& win = MainLoop::get_window();
    auto& st = store();
    auto& bat = batcher();

    st.clear();

    for (auto [e, sprite] : reg.view<cmp::Sprite>().each()) {
        auto* s = sprite.sprite_ptr();
        if (!s) continue;

        int z = 0;
        if (auto* zi = reg.try_get<cmp::ZIndex>(e)) z = zi->value;

        st.flat.push_back({s, z});
    }

    if (st.flat.empty()) return;

    for (auto& e : st.flat) st.zs_unique.push_back(e.z);
    std::sort(st.zs_unique.begin(), st.zs_unique.end());
    st.zs_unique.erase(
        std::unique(st.zs_unique.begin(), st.zs_unique.end()),
        st.zs_unique.end()
    );

    std::unordered_map<int, uint32_t> z_index_of;
    for (size_t i = 0; i < st.zs_unique.size(); ++i)
        z_index_of[st.zs_unique[i]] = static_cast<uint32_t>(i);

    std::vector<uint32_t> counts(st.zs_unique.size(), 0);
    for (auto& e : st.flat) counts[z_index_of[e.z]]++;

    std::vector<uint32_t> offsets(st.zs_unique.size());
    uint32_t total = 0;
    for (size_t i = 0; i < counts.size(); ++i) {
        offsets[i] = total;
        total += counts[i];
    }

    std::vector<RenderEntry> ordered(st.flat.size());
    std::vector<uint32_t> cursor = offsets;
    for (auto& e : st.flat)
        ordered[cursor[z_index_of[e.z]]++] = e;

    bat.begin();
    for (auto& e : ordered) bat.add(*e.sprite);
    bat.flush(win);
}

}

ME_REGISTER_SYSTEM(
    "engine:sprite:render",
    me::SystemPhase::Render,
    std::numeric_limits<int>::min() + 6,
    sprite_render
)