#include <meatengine/MainLoop.hpp>

#include <meatengine/Resources.hpp>
#include <meatengine/ResourceLoader.hpp>
#include <meatengine/console/Console.hpp>
#include <meatengine/Generic.hpp>

#include <meatengine/ui/Components.hpp>
#include <meatengine/ui/Systems.hpp>

#include <meatengine/render/Systems.hpp>
#include <meatengine/sprite/Systems.hpp>
#include <meatengine/camera/Systems.hpp>
#include <meatengine/timer/Systems.hpp>
#include <meatengine/tilemap/Systems.hpp>
#include <meatengine/systems/Common.hpp>

#include <meatengine/ScriptingServer.hpp>

namespace me {

    std::string MainLoop::get_window_title() { return m_window_title; }
    void MainLoop::set_window_title(std::string& new_title) {
        m_window_title = new_title;
        m_window.setTitle(m_window_title);
    }

    sf::RenderWindow& MainLoop::get_window() { return m_window; }
    const sf::RenderWindow& MainLoop::get_window() const { return m_window; }

    sf::Clock& MainLoop::get_clock() { return m_clock; }
    const sf::Clock& MainLoop::get_clock() const { return m_clock; }

    entt::registry& MainLoop::get_registry() { return m_registry; }
    const entt::registry& MainLoop::get_registry() const { return m_registry; }

    void MainLoop::set_framerate_limit(float value) {
        m_target_fps = value;
        m_window.setFramerateLimit(m_target_fps);
    }
    float MainLoop::get_framerate_limit() { return m_target_fps; }
    float MainLoop::get_fps() { return m_fps; }

    void MainLoop::update_engine(float dt) {
        me::systems::movement(m_registry, dt);
        TimerSystems::update(m_registry, dt);
        TileMapSystems::update(m_registry);
        me::SpriteSystems::update(m_registry, m_window, dt);
        me::ui::Systems::process_events(m_registry, m_window);
		me::ui::Systems::update(m_registry);
        me::CameraSystems::update(m_registry, m_window, dt);

        me::Console::get_instance().update(m_window, dt);
    }

    void MainLoop::render_engine() {
        RenderSystems::render(m_registry, m_window);
    }

    void MainLoop::render_engine_default_view() {
        RenderSystems::render_default_view(m_registry, m_window);
    }

    MainLoop::MainLoop(const std::string& title, sf::VideoMode default_mode) 
        : m_prev_mode(default_mode), m_window_title(title) 
    {
        m_registry.ctx().emplace<InputState>();
        m_window.create(m_prev_mode, title);
        m_window.setFramerateLimit(144);

        ScriptingServer::lua().set_function("get_global_registry",
            [this]() -> entt::registry& { return m_registry; }
        );
        ScriptingServer::lua().set_function("get_fps",
            [this]() -> float { return m_fps; }
        );
        Generic::updating::install(m_registry);
        ui::updating::install(m_registry);

        // потом как нибудь
        entt::resource<me::Font> mainfont = me::ResourceLoader::load<Font>("data/core/res/fonts/mainfont.ttf");
        if (mainfont.handle()) {
            Console::get_instance().init(*this, mainfont->res, 16);
        } else {
            std::shared_ptr<me::Font> default_font = ResourceLoader::get_default<Font>();
            if (default_font != nullptr) {
                Console::get_instance().init(*this, default_font->res, 16);
            }
        }
        //
    }

    void MainLoop::change_state(std::unique_ptr<GameState> new_state) {
        m_next_state = std::move(new_state);
    }

    void MainLoop::process_events() {
        while (const std::optional event = m_window.pollEvent()) {
            auto& in = m_registry.ctx().get<InputState>();

            in.mouse_pos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
            in.mouse_down = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            in.mouse_just_pressed = false;
            in.mouse_just_released = false;

            if (event->is<sf::Event::Closed>()) {
                m_window.close();
            }
            else if (auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Left)
                in.mouse_just_pressed = true;
            }
            else if (auto* mb = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mb->button == sf::Mouse::Button::Left)
                    in.mouse_just_released = true;
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::F11) {
                    m_fullscreen = !m_fullscreen;
                    if (m_fullscreen) {
                        m_window.create(sf::VideoMode::getDesktopMode(), m_window_title, sf::State::Fullscreen);
                    } else {
                        m_window.create(m_prev_mode, m_window_title, sf::State::Windowed);
                    }
                    m_window.setFramerateLimit(m_target_fps);
                }
            }

            // потом как нибудь
            me::Console::get_instance().handle_event(*event, m_window);
            //

            if (m_current_state) {
                m_current_state->handle_event(m_window, m_registry, *event);
            }
        }
    }

    void MainLoop::run(std::unique_ptr<GameState> initial_state) {
        change_state(std::move(initial_state));

        while (m_window.isOpen()) {
            if (m_next_state) {
                if (m_current_state) m_current_state->on_exit(m_window, m_registry);
                m_registry.clear();
                m_current_state = std::move(m_next_state);
                m_current_state->on_enter(m_window, m_registry);
            }

            process_events();

            float dt = m_clock.restart().asSeconds();
            float scaled_dt = dt * dt_scale;

            m_fps_accum  += dt;
            m_fps_frames += 1;
            if (m_fps_accum >= m_fps_update_interval) {
                m_fps = static_cast<float>(m_fps_frames) / m_fps_accum;
                m_fps_accum  = 0.f;
                m_fps_frames = 0;
            }

            if (m_current_state) {
                m_current_state->update(m_window, m_registry, scaled_dt);
            }
            update_engine(scaled_dt);
            m_current_state->update_deferred(m_window, m_registry, scaled_dt);

            m_window.clear(sf::Color::Black);
            
            if (m_current_state) {
                m_current_state->render(m_window, m_registry, scaled_dt);
            }
            render_engine();
            m_current_state->render_deferred(m_window, m_registry, scaled_dt);

            m_window.setView(m_window.getDefaultView());

            render_engine_default_view();
            m_current_state->render_default_view(m_window, m_registry, scaled_dt);

            me::Console::get_instance().render(m_window);



            m_window.display();
        }
    }

} // namespace me
 