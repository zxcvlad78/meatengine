#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <memory>
#include "GameState.hpp"

namespace me {
    class MainLoop {
    public:
        MainLoop(const std::string& title, sf::VideoMode default_mode = sf::VideoMode{sf::Vector2u(1280, 720)});
        ~MainLoop() = default;

        void run(std::unique_ptr<GameState> initial_state);
        void change_state(std::unique_ptr<GameState> new_state);

        std::string get_window_title();
        void set_window_title(std::string& new_title);

        sf::RenderWindow& get_window();
        const sf::RenderWindow& get_window() const;

        sf::Clock& get_clock();
        const sf::Clock& get_clock() const;

        entt::registry& get_registry();
        const entt::registry& get_registry() const;

        void set_framerate_limit(float value);
        float get_framerate_limit();

        inline static float dt_scale = 1.f;

    private:
        void process_events();
        void update_engine(float dt);
        void render_engine();

        std::string m_window_title;
        sf::RenderWindow m_window;
        bool m_fullscreen = false;
        sf::VideoMode m_prev_mode;

        int m_target_fps = 144;

        sf::Clock m_clock;
        entt::registry m_registry;
        
        std::unique_ptr<GameState> m_current_state = nullptr;
        std::unique_ptr<GameState> m_next_state = nullptr;

        float m_fps = 0.f;
        float m_fps_accum = 0.f;
        int m_fps_frames = 0;
        static constexpr float m_fps_update_interval = 0.25f;
    };
} // namespace me
