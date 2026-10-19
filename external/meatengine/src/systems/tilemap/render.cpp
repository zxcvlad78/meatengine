#include <meatengine/SystemRegistry.hpp>
#include <meatengine/MainLoop.hpp>

#include <meatengine/cmp/Transform.hpp>
#include <meatengine/cmp/TileMap.hpp>

namespace me {

static void rebuild_vertices(me::cmp::TileMap& tilemap) {
    auto& out = tilemap.cpu_vertices;
    out.clear();

    if (!tilemap.tileset || tilemap.tiles.empty()) return;

    const unsigned short tile_size = tilemap.tileset->tile_size;
    const sf::Vector2u texture_size = tilemap.tileset->size();
    if (tile_size == 0 || texture_size.x == 0 || texture_size.y == 0) return;

    const unsigned int tiles_per_row = texture_size.x / tile_size;
    if (tiles_per_row == 0) return;

    const unsigned int w = tilemap.width;
    const unsigned int h = tilemap.height;
    if (w == 0 || h == 0) return;

    out.reserve(static_cast<std::size_t>(w) * h * 6);

    const int tile_size_i = static_cast<int>(tile_size);

    for (unsigned int ly = 0; ly < h; ++ly) {
        const unsigned int row_off = ly * w;
        const int wy = tilemap.origin_y + static_cast<int>(ly);
        const float py = static_cast<float>(wy * tile_size_i);

        for (unsigned int lx = 0; lx < w; ++lx) {
            const int tile_id = tilemap.tiles[row_off + lx];
            if (tile_id < 0) continue;

            const unsigned int tx = static_cast<unsigned int>(tile_id) % tiles_per_row;
            const unsigned int ty = static_cast<unsigned int>(tile_id) / tiles_per_row;

            const float left = static_cast<float>(tx * tile_size);
            const float top = static_cast<float>(ty * tile_size);
            const float right = left + static_cast<float>(tile_size);
            const float bottom = top  + static_cast<float>(tile_size);

            const int wx = tilemap.origin_x + static_cast<int>(lx);
            const float px = static_cast<float>(wx * tile_size_i);

            out.emplace_back(sf::Vertex{{px, py}, sf::Color::White, {left, top} });
            out.emplace_back(sf::Vertex{{px + static_cast<float>(tile_size), py}, sf::Color::White, {right, top}});
            out.emplace_back(sf::Vertex{{px, py + static_cast<float>(tile_size)}, sf::Color::White, {left, bottom}});

            out.emplace_back(sf::Vertex{{px + static_cast<float>(tile_size), py}, sf::Color::White, {right, top}});
            out.emplace_back(sf::Vertex{{px + static_cast<float>(tile_size), py + static_cast<float>(tile_size)}, sf::Color::White, {right, bottom}});
            out.emplace_back(sf::Vertex{{px, py + static_cast<float>(tile_size)}, sf::Color::White, {left, bottom}});
        }
    }
}

static void render() {
    auto& reg = MainLoop::get_registry();
    auto& window = MainLoop::get_window();

    auto view = reg.view<me::cmp::TileMap, me::cmp::Transform>();

    for (auto [entity, tilemap, transform] : view.each()) {
        if (!tilemap.tileset) continue;
        if (tilemap.tiles.empty()) continue;

        if (tilemap.dirty) {
            rebuild_vertices(tilemap);
            tilemap.dirty = false;

            const std::size_t count = tilemap.cpu_vertices.size();

            if (count == 0) {
                static_cast<void>(tilemap.vertex_buffer.create(0));
                continue;
            }

            if (tilemap.vertex_buffer.getVertexCount() != count) {
                tilemap.vertex_buffer.setPrimitiveType(sf::PrimitiveType::Triangles);
                static_cast<void>(tilemap.vertex_buffer.create(count));
            }

            static_cast<void>(tilemap.vertex_buffer.update(tilemap.cpu_vertices.data()));
        }

        if (tilemap.vertex_buffer.getVertexCount() == 0) continue;

        sf::Transform t;
        t.translate(transform.position);
        sf::RenderStates states(t);
        states.texture = &tilemap.tileset->texture->res;

        window.draw(tilemap.vertex_buffer, states);
    }
}

}

ME_REGISTER_SYSTEM(
    "engine:tilemap:render",
    me::SystemPhase::Render,
    std::numeric_limits<int>::min() + 3,
    me::render
)