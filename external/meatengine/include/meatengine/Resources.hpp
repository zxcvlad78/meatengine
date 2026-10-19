#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <vector>
#include <map>
#include <string>

#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"
#include <entt/resource/cache.hpp>
#include <meatengine/parsing.hpp>

using json = nlohmann::json;

namespace me {
    inline json get_json_data(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return nullptr;
        try {
            json j;
            file >> j;
            return j;
        } catch (const json::exception& e) {
            std::cerr << "get_json_data: " << path << ": " << e.what() << '\n';
            return nullptr;
        }
    }

    inline bool get_json_data(const std::string& path, json& target_json) {
        target_json = get_json_data(path);

        return !target_json.is_null();
    }

    struct Font {
        using result_type = std::shared_ptr<Font>;

        sf::Font res;
        std::shared_ptr<Font> operator()(const std::string& path) const;
    };

    struct Texture {
        using result_type = std::shared_ptr<Texture>;

        sf::Texture res;
        std::shared_ptr<Texture> operator()(const std::string& path) const;
    };



    struct SoundBuffer {
        using result_type = std::shared_ptr<SoundBuffer>;

        sf::SoundBuffer res;
        std::shared_ptr<SoundBuffer> operator()(const std::string& path) const;
    };

    struct Shader {
        using result_type = std::shared_ptr<Shader>;

        sf::Shader res;
        std::shared_ptr<Shader> operator()(const std::string& vertex_path, const std::string& fragment_path) const;
    };

    struct Animation {
        using result_type = std::shared_ptr<Animation>;

        struct FrameData {
            int x, y, w, h;
        };

        std::string name;
        float fps = 12.f;
        bool is_looping = true;
        std::vector<FrameData> frames;

        float duration() const {
            if (fps == 0.0f) return 0.0f;
            int size = frames.size();
            if (size == 0) return 0.0f;

            return size / fps;
        }

    };

    struct SpriteSheet {
        using result_type = std::shared_ptr<SpriteSheet>;

        int atlas_width = 1;
        int atlas_height = 1;
        std::unordered_map<std::string, Animation> animations;

        std::shared_ptr<SpriteSheet> operator()(const std::string& path) const;
    };

    struct TileSet {
        using result_type = std::shared_ptr<TileSet>;

        entt::resource<Texture> texture;
        sf::Vector2u size() {
            if (texture.handle() == nullptr) return sf::Vector2u{ 0, 0 };
            return texture->res.getSize();
        }
        
        unsigned short tile_size = 16;
        short y_sort_origin = 0;

        std::shared_ptr<TileSet> operator()(const std::string& path) const;
    };

    struct StyleBox {
        using result_type = std::shared_ptr<StyleBox>;

        json data;
        
        template<typename T>
        T get_value(const std::string& key, T default_value) const {
            if (!data.is_object()) return default_value;

            auto it = data.find(key);
            if (it == data.end() || it->is_null()) return default_value;

            try { return it->get<T>(); }
            catch (const json::exception&) { return default_value; }
        }

        template<typename T>
        void set_value(const std::string& key, T value) {
            if (!data.is_object()) data = json::object();
                data[key] = std::move(value);
        }

        std::shared_ptr<StyleBox> operator()(const std::string& path) const;
    };
}