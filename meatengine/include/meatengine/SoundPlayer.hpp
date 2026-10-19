#pragma once

#include <meatengine/ResourceLoader.hpp>
#include <meatengine/Resources.hpp>

#undef max

namespace me {
    class SoundPlayer {
    private:
        inline static sf::SoundBuffer dummy_buffer;

        struct Slot {
            sf::Sound sound;
            entt::resource<SoundBuffer> buffer;
            int priority = 0;

            Slot() : sound(dummy_buffer) {}
            Slot(const sf::SoundBuffer& buffer) : sound(buffer) {}
        };

        static constexpr size_t POOL_SIZE = 32;
        inline static std::vector<Slot> pool;
        inline static bool initialized;

        static void initialize() {
            if (initialized) return;
            pool.resize(POOL_SIZE);
            initialized = true;
        }

    public:
        SoundPlayer() = delete;

        static void play(
            entt::resource<SoundBuffer> buffer,
            sf::Vector2f position = {0.f, 0.f},
            float min_distance = 100.f,
            int priority = 0,
            float attenuation = 1.f
        ) {
            initialize();
            if (!buffer) return;

            int slot = -1;
            int lowest_priority = std::numeric_limits<int>::max();

            for (size_t i = 0; i < pool.size(); ++i) {
                if (pool[i].sound.getStatus() == sf::Sound::Status::Stopped) {
                    slot = static_cast<int>(i);
                    break;
                }
                if (pool[i].priority < lowest_priority) {
                    lowest_priority = pool[i].priority;
                    slot = static_cast<int>(i);
                }
            }

            if (slot == -1) return;

            if (pool[slot].sound.getStatus() != sf::Sound::Status::Stopped &&
                priority <= pool[slot].priority) {
                return;
            }

            pool[slot].sound.stop();
            pool[slot].buffer = buffer;
            pool[slot].sound.setBuffer(buffer->res);
            pool[slot].sound.setPosition({position.x, position.y, 0.f});
            pool[slot].sound.setRelativeToListener(false);
            pool[slot].sound.setMinDistance(min_distance);
            pool[slot].sound.setAttenuation(attenuation);
            pool[slot].priority = priority;
            pool[slot].sound.play();
        }

        static void stop_all() {
            initialize();
            for (auto& slot : pool) {
                slot.sound.stop();
                slot.buffer.reset();
            }
        }
    };

    // std::vector<SoundPlayer::Slot> SoundPlayer::pool;
    // bool SoundPlayer::initialized = false;

} // namespace me