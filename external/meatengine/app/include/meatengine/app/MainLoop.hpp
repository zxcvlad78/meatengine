#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "GameState.hpp"

namespace me {
    class MainLoop {
    public:
        MainLoop() = delete;

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

        static float get_dt_scale();
        static void set_dt_scale(float dt);

        static float get_dt();

        static void set_fixed_dt(float seconds);
        static float get_fixed_dt();
        static float get_fixed_alpha();

        static void set_max_fixed_steps(int n);
        static int  get_max_fixed_steps();

        using InitHook     = std::function<void(sf::RenderWindow&)>;
        using UpdateHook   = std::function<void(sf::RenderWindow&, float)>;
        using RenderHook   = std::function<void(sf::RenderWindow&)>;
        using EventHook    = std::function<void(const sf::Event&, sf::RenderWindow&)>;
        using ShutdownHook = std::function<void()>;

        static void add_on_init(InitHook hook);
        static void add_on_update(UpdateHook hook);
        static void add_on_render(RenderHook hook);
        static void add_on_event(EventHook hook);
        static void add_on_shutdown(ShutdownHook hook);

    private:
        static void process_events();

        inline static std::string m_window_title;
        inline static sf::RenderWindow m_window;
        inline static bool m_fullscreen = false;
        inline static sf::VideoMode m_prev_mode;

        inline static int m_target_fps = 144;

        inline static sf::Clock m_clock;
        inline static entt::registry m_registry;

        inline static std::unique_ptr<GameState> m_current_state = nullptr;
        inline static std::unique_ptr<GameState> m_next_state = nullptr;

        inline static float m_dt = 0.f;
        inline static float m_dt_scale = 1.f;

        inline static float m_fixed_dt = 1.f / 60.f;
        inline static int m_max_fixed_steps = 8;
        inline static float m_fixed_accum = 0.f;
        inline static float m_fixed_alpha = 0.f;

        inline static std::vector<InitHook> m_on_init;
        inline static std::vector<UpdateHook> m_on_update;
        inline static std::vector<RenderHook> m_on_render;
        inline static std::vector<EventHook> m_on_event;
        inline static std::vector<ShutdownHook> m_on_shutdown;
    };
} // namespace me
