#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>

namespace me {

class SpriteBatcher {
public:
    void begin() {
        for (auto& [_, b] : batches) b.vertices.clear();
        live_batches.clear();
        total = 0;
    }

    void add(const sf::Sprite& sprite) {
        const sf::Texture& tex = sprite.getTexture();

        auto& batch = batches[&tex];
        if (batch.vertices.empty()) {
            batch.texture = &tex;
            live_batches.push_back(&batch);
        }

        const sf::Transform& xf = sprite.getTransform();
        const sf::IntRect& r = sprite.getTextureRect();
        const sf::Color c = sprite.getColor();

        const float w = static_cast<float>(r.size.x);
        const float h = static_cast<float>(r.size.y);
        const float l = static_cast<float>(r.position.x);
        const float t = static_cast<float>(r.position.y);
        const float rr = l + w;
        const float bb = t + h;

        auto push = [&](float x, float y, float u, float v) {
            sf::Vertex vert;
            vert.position = xf.transformPoint({x, y});
            vert.texCoords = {u, v};
            vert.color = c;
            batch.vertices.push_back(vert);
        };

        push(0.f, 0.f, l, t);
        push(w, 0.f, rr, t);
        push(0.f, h, l, bb);
        push(w, 0.f, rr, t);
        push(w, h, rr, bb);
        push(0.f, h, l, bb);

        ++total;
    }

    void flush(sf::RenderTarget& target) {
        for (auto* b : live_batches) {
            if (b->vertices.empty()) continue;

            sf::RenderStates states;
            states.texture = b->texture;

            target.draw(
                b->vertices.data(),
                b->vertices.size(),
                sf::PrimitiveType::Triangles,
                states
            );
        }
    }

    std::size_t size() const noexcept { return total; }

private:
    struct Batch {
        const sf::Texture* texture = nullptr;
        std::vector<sf::Vertex> vertices;
    };

    std::unordered_map<const sf::Texture*, Batch> batches;
    std::vector<Batch*> live_batches;
    std::size_t total = 0;
};

} 