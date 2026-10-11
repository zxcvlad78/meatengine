#include <meatengine/MainLoop.hpp>

#include <meatengine/Resources.hpp>
#include <meatengine/ResourceLoader.hpp>
#include <meatengine/console/Console.hpp>
#include <meatengine/ctx/InputState.hpp>

#include <meatengine/ScriptingServer.hpp>
#include <meatengine/SystemRegistry.hpp>
#include <meatengine/registry_setup.hpp>

namespace me {

    void MainLoop::init(const std::string& title, sf::VideoMode default_mode) {
        m_prev_mode = std::move(default_mode);
        m_window_title = std::move(title);
        m_registry.ctx().emplace<ctx::InputState>();
        me::registry_setup::run(m_registry);
        m_window.create(m_prev_mode, title);
        m_window.setFramerateLimit(144);

        // потом как нибудь
        entt::resource<me::Font> mainfont = me::ResourceLoader::load<Font>("data/core/res/fonts/mainfont.ttf");
        if (mainfont.handle()) {
            Console::get_instance().init(mainfont->res, 16);
        } else {
            std::shared_ptr<me::Font> default_font = ResourceLoader::get_default<Font>();
            if (default_font != nullptr) {
                Console::get_instance().init(default_font->res, 16);
            }
        }
        //
    }

    std::string MainLoop::get_window_title() { return m_window_title; }
    void MainLoop::set_window_title(std::string& new_title) {
        m_window_title = new_title;
        m_window.setTitle(m_window_title);
    }
    void MainLoop::set_window_title(const std::string& new_title) {
        m_window_title = new_title;
        m_window.setTitle(m_window_title);
    }

    void MainLoop::change_state(std::unique_ptr<GameState> new_state) {
        m_next_state = std::move(new_state);
    }

    sf::RenderWindow& MainLoop::get_window() { return m_window; }
    sf::Clock& MainLoop::get_clock() { return m_clock; }
    entt::registry& MainLoop::get_registry() { return m_registry; }

    void MainLoop::set_framerate_limit(float value) {
        m_target_fps = value;
        m_window.setFramerateLimit(m_target_fps);
    }
    float MainLoop::get_framerate_limit() { return m_target_fps; }
    float MainLoop::get_dt() { return m_dt; }

    float MainLoop::get_dt_scale() { return m_dt_scale; }
    void MainLoop::set_dt_scale(float dt) { m_dt_scale = dt; }

    void MainLoop::set_fixed_dt(float seconds) {
        if (seconds <= 0.f) {
            std::cerr << "[MainLoop] set_fixed_dt: invalid value\n";
            return;
        }
        m_fixed_dt = seconds;
    }

    float MainLoop::get_fixed_dt() { return m_fixed_dt; }
    float MainLoop::get_fixed_alpha() { return m_fixed_alpha; }

    void MainLoop::set_max_fixed_steps(int n) {
        if (n < 1) return;
        m_max_fixed_steps = n;
    }

    int MainLoop::get_max_fixed_steps() { return m_max_fixed_steps; }


    void MainLoop::process_events() {
        auto& in = m_registry.ctx().get<ctx::InputState>();

        in.mouse_just_pressed = false;
        in.mouse_just_released = false;
        in.mouse_down = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        in.mouse_pos = m_window.mapPixelToCoords(
            sf::Mouse::getPosition(m_window),
            m_window.getDefaultView()
        );

        while (const std::optional event = m_window.pollEvent()) {

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

            m_dt = m_clock.restart().asSeconds();

            me::SystemRegistry::run(SystemPhase::Input);

            m_fixed_accum += m_dt;
            int steps = 0;
            while (m_fixed_accum >= m_fixed_dt && steps < m_max_fixed_steps) {
                me::SystemRegistry::run(SystemPhase::FixedUpdate);
                
                m_fixed_accum -= m_fixed_dt;
                ++steps;
            }
            
            if (m_fixed_accum >= m_fixed_dt) {
                m_fixed_accum = 0.f;
            }
            m_fixed_alpha = m_fixed_accum / m_fixed_dt;

            me::SystemRegistry::run(SystemPhase::Update);
            me::Console::get_instance().update(m_window, m_dt); // remove

            m_window.clear(sf::Color::Black);
            
            me::SystemRegistry::run(SystemPhase::Render);

            m_window.setView(m_window.getDefaultView());

            me::SystemRegistry::run(SystemPhase::PostRender);
            me::Console::get_instance().render(m_window); // remove

            m_window.display();
        }
    }

} // namespace me
 