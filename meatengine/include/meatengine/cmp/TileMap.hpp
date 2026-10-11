#pragma once
#include <meatengine/Resources.hpp>
#include <SFML/Graphics.hpp>

namespace me::cmp {
	struct TileMap {
        int origin_x = 0;
        int origin_y = 0;

		unsigned int width;
		unsigned int height;
		
		bool dirty = true;

		std::vector<int> tiles;
		entt::resource<me::TileSet> tileset;
	
		sf::VertexBuffer vertex_buffer{
			sf::PrimitiveType::Triangles,
			sf::VertexBuffer::Usage::Static
		};
		std::vector<sf::Vertex> cpu_vertices;

        void load_tiles(const std::string& path) {
            json data = get_json_data(path);
            if (data.contains("width")) width = data["width"];
            if (data.contains("height")) height = data["height"];
            if (data.contains("tiles")) tiles  = data["tiles"].get<std::vector<int>>();

            origin_x = 0;
            origin_y = 0;

            const std::size_t needed = static_cast<std::size_t>(width) * height;
            if (tiles.size() < needed) tiles.resize(needed, -1);

            dirty = true;
        }

        void set_tile(int x, int y, int tile_idx) {
            int new_left = origin_x;
            int new_top = origin_y;
            int new_right = origin_x + static_cast<int>(width);
            int new_bottom = origin_y + static_cast<int>(height);

            if (width == 0 || height == 0) {
                new_left = x;
                new_top = y;
                new_right = x + 1;
                new_bottom = y + 1;
            } else {
                if (x < new_left) new_left = x;
                if (y < new_top) new_top = y;
                if (x >= new_right) new_right = x + 1;
                if (y >= new_bottom) new_bottom = y + 1;
            }

            const unsigned int new_width  = static_cast<unsigned int>(new_right  - new_left);
            const unsigned int new_height = static_cast<unsigned int>(new_bottom - new_top);

            if (new_left != origin_x || new_top != origin_y ||
                new_width != width || new_height != height) {
					std::vector<int> new_tiles(
						static_cast<std::size_t>(new_width) * new_height, -1
					);

					for (unsigned int ly = 0; ly < height; ++ly) {
						for (unsigned int lx = 0; lx < width; ++lx) {
							const int wx = origin_x + static_cast<int>(lx);
							const int wy = origin_y + static_cast<int>(ly);

							const unsigned int nlx = static_cast<unsigned int>(wx - new_left);
							const unsigned int nly = static_cast<unsigned int>(wy - new_top);

							new_tiles[static_cast<std::size_t>(nly) * new_width + nlx] = tiles[
								static_cast<std::size_t>(ly) * width + lx
							];
						}
					}

					tiles = std::move(new_tiles);
					width = new_width;
					height = new_height;
					origin_x = new_left;
					origin_y = new_top;
				}

            const unsigned int lx = static_cast<unsigned int>(x - origin_x);
            const unsigned int ly = static_cast<unsigned int>(y - origin_y);
            tiles[static_cast<std::size_t>(ly) * width + lx] = tile_idx;
            dirty = true;
        }

        int get_tile(int x, int y) const {
            if (x < origin_x) return -1;
            if (y < origin_y) return -1;
            if (x >= origin_x + static_cast<int>(width))  return -1;
            if (y >= origin_y + static_cast<int>(height)) return -1;

            const unsigned int lx = static_cast<unsigned int>(x - origin_x);
            const unsigned int ly = static_cast<unsigned int>(y - origin_y);
            return tiles[static_cast<std::size_t>(ly) * width + lx];
        }
	};
}