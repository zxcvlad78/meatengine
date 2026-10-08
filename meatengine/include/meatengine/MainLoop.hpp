#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <memory>
#include "GameState.hpp"

namespace me {
    class MainLoop {
    public:
        MainLoop() = delete;
        //~MainLoop() = default;

        static void init(const std::string& title, sf::VideoMode default_mode = sf::VideoMode{sf::Vector2u(1280, 720)});

        static void run(std::unique_ptr<GameState> initial_state);

        static void change_state(std::unique_ptr<GameState> new_state);

        template<typename T, typename... Args>
        static void change_state(Args&&... args) {
            static_assert(std::is_base_of_v<GameState, T>, "T must derive from GameState");
            m_next_state = std::make_unique<T>(std::forward<Args>(args)...);
        }

        static std::string get_window_title();
        static void set_window_title(std::string& new_title);
        static void set_window_title(const std::string& new_title);

        static sf::RenderWindow& get_window();
        static sf::Clock& get_clock();
        static entt::registry& get_registry();

        static void set_framerate_limit(float value);
        static float get_framerate_limit();

        static float get_fps();
        inline static float dt_scale = 1.f;

    private:
        static void process_events();
        static void update_engine(float dt);
        static void render_engine();
        static void render_engine_default_view();

        inline static std::string m_window_title;
        inline static sf::RenderWindow m_window;
        inline static bool m_fullscreen = false;
        inline static sf::VideoMode m_prev_mode;

        inline static int m_target_fps = 144;

        inline static sf::Clock m_clock;
        inline static entt::registry m_registry;
        
        inline static std::unique_ptr<GameState> m_current_state = nullptr;
        inline static std::unique_ptr<GameState> m_next_state = nullptr;

        inline static float m_fps = 0.f;
        inline static float m_fps_accum = 0.f;
        inline static int m_fps_frames = 0;
        static constexpr float m_fps_update_interval = 0.5f;
    };
} // namespace me
