#include <meatengine/console/Console.hpp>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <cctype>
#include <meatengine/parsing.hpp>
#include <meatengine/console/commands.hpp>
#include <meatengine/FileSystem.hpp>

namespace me {
    static int lua_print(lua_State* L) {
        int n = lua_gettop(L);
        std::string result;
        for (int i = 1; i <= n; ++i) {
            size_t len = 0;
            const char* s = luaL_tolstring(L, i, &len);
            if (i > 1) result += '\t';
            result.append(s, len);
            lua_pop(L, 1);
            std::cout << s << std::endl;
        }
        Console::get_instance().print(result);
        return 0;
    }

    Console& Console::get_instance() {
        static Console instance;
        return instance;
    }

    void Console::save_history() {
        std::string out;
        out.reserve(command_history.size() * 32);

        for (const auto& cmd : command_history) {
            for (char c : cmd) out += (c == '\n' || c == '\r') ? ' ' : c;
            out += '\n';
        }

        const std::string path = history_file_path;
        me::FileSystem::save_file(path, out);
    }

    void Console::load_history() {
        command_history.clear();

        std::string data;
        if (!me::FileSystem::get_file_text(history_file_path, data)) {
            history_index = -1;
            return;
        }

        std::istringstream stream(data);
        std::string line;
        while (std::getline(stream, line)) {
            if (!line.empty()) command_history.push_back(std::move(line));
        }

        while (command_history.size() > max_history)
            command_history.pop_front();

        history_index = -1;
    }

    void Console::update_ui_cfg(sf::RenderWindow& window) {
        sf::Vector2f window_size = static_cast<sf::Vector2f>(window.getSize());

        theme_outline_color = me::parsing::str_to_color(config_file->get("theme_outline_color", "30 30 40"));
        foreground_color = me::parsing::str_to_color(config_file->get("foreground_color", "25 25 25 255"));
        background_color = me::parsing::str_to_color(config_file->get("background_color", "15 15 15 240"));
        selection_color = me::parsing::str_to_color(config_file->get("selection_color", "255 15 15 127"));
        char_size = config_file->get("char_size", 16);
        console_width = config_file->get("console_width", window_size.x);
        console_height = config_file->get("console_height", window_size.y);
        input_padding = config_file->get("input_padding", 10.f);
        theme_outline_thickness = config_file->get("theme_outline_thickness", .1f);
    }
    void Console::reset_cfg(sf::RenderWindow& window, bool update_ui) {
        config_file->set("theme_outline_color", "30 30 40");
        config_file->set("foreground_color", "25 25 25 255");
        config_file->set("background_color", "15 15 15 240");
        config_file->set("selection_color", "255 15 15 127");
        config_file->set("char_size", 14);
        config_file->set("console_width", window.getSize().x);
        config_file->set("console_height", window.getSize().y);
        config_file->set("input_padding", 10.f);
        config_file->set("theme_outline_thickness", .1f);

        config_file->save();

        texts_dirty = true;
        if (update_ui) update_ui_cfg(window);
    }
    void Console::load_cfg(sf::RenderWindow& window) {
        if (!config_file->load()) reset_cfg(window, false);

        update_ui_cfg(window);
        texts_dirty = true;
    }

    inline bool is_word_char(char ch) { return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_'; }

    inline int prev_word_position(int pos, const std::string& str) {
        if (pos <= 0) return 0;
        int i = pos - 1;
        while (i >= 0 && !is_word_char(str[i])) --i;
        while (i >= 0 && is_word_char(str[i])) --i;
        return i + 1;
    }

    inline int next_word_position(int pos, const std::string& str) {
        int len = str.size();
        if (pos >= len) return len;
        int i = pos;
        while (i < len && !is_word_char(str[i])) ++i;
        while (i < len && is_word_char(str[i])) ++i;
        return i;
    }


    void Console::init(sf::Font& f, uint16_t character_size) {
        if (inited) return;

        ScriptingServer::lua().set_function("print", lua_print);
        load_cfg(MainLoop::get_window());
        load_history();
        font_ptr = &f;
        char_size = character_size;

        input_rect_text = std::make_unique<sf::Text>(f);
        head_title = std::make_unique<sf::Text>(f);
        suggestion_text = std::make_unique<sf::Text>(f);
        fps_text = std::make_unique<sf::Text>(f);

        selection_rect.setFillColor(selection_color);

        background_rect.setFillColor(background_color);
        background_rect.setOutlineThickness(theme_outline_thickness);
        background_rect.setOutlineColor(theme_outline_color);

        head_rect.setFillColor(foreground_color);
        head_rect.setOutlineThickness(theme_outline_thickness);
        head_rect.setOutlineColor(theme_outline_color);

        head_title->setCharacterSize(24);
        head_title->setFillColor(sf::Color(200, 180, 180));
        head_title->setString("me::Console");

        input_rect.setFillColor(foreground_color);
        input_rect.setOutlineThickness(theme_outline_thickness);
        input_rect.setOutlineColor(theme_outline_color);

        input_rect_text->setCharacterSize(char_size);
        input_rect_text->setFillColor(sf::Color(220, 220, 220));

        suggestion_rect.setFillColor(foreground_color);
        suggestion_text->setCharacterSize(char_size - 2);
        suggestion_text->setFillColor(sf::Color(150, 150, 150, 200));

        scrollbar_track.setFillColor(sf::Color(40, 40, 45, 150));
        scrollbar_thumb.setFillColor(sf::Color(100, 100, 110, 200));

        cursor_rect.setFillColor(sf::Color(255, 255, 255, 200));
        cursor_rect.setSize({2.f, static_cast<float>(char_size)});

        register_default_commands();
        console_commands::init();
        print_success("Console initialized! Type 'help' for available commands");
        texts_dirty = true;
        inited = true;
    }

    void Console::show() { visible = true; }
    void Console::hide() { visible = false; }
    void Console::toggle() { visible = !visible; }

    void Console::print(const std::string& text) {
        auto fragments = BBCode::parse(text, char_size);
        messages.push_back(fragments);
        if (messages.size() > max_messages) messages.pop_front();
        texts_dirty = true;
    }
 
    void Console::print_plain(const std::string& text, sf::Color color) {
        std::vector<BBCode::TextFragment> frags;
        frags.emplace_back(text, color, char_size, false, false, false);
        messages.push_back(frags);
        if (messages.size() > max_messages) messages.pop_front();
        texts_dirty = true;
    }

    void Console::register_command(const std::string& name,
        std::function<void(const std::vector<std::string>&)> handler,
        const std::string& description,
        const std::string& usage
    ) {  commands[name] = {name, handler, description, usage}; }

    void Console::unregister_command(const std::string& name) {
        commands.erase(name);
    }

    void Console::execute_command(const std::string& command_line) {
        if (command_line.empty()) return;
        add_to_history(command_line);

        if (echo_mode) {
            print_plain("> " + command_line, sf::Color(150, 150, 150));
        }

        std::istringstream iss(command_line);
        std::string cmd_name;
        iss >> cmd_name;

        auto it = commands.find(cmd_name);
        if (it != commands.end()) {
            std::vector<std::string> args;
            std::string arg;
            while (iss >> arg) args.push_back(arg);
            try {
                it->second.handler(args);
            } catch (const std::exception& e) {
                print_error("Command execution failed: " + std::string(e.what()));
            }
        } else {
            print_error("Unknown command: " + cmd_name);
        }
    }

    std::vector<std::string> Console::get_suggestions(const std::string& prefix) {
        std::vector<std::string> suggestions;
        for (const auto& [name, cmd] : commands) {
            if (name.find(prefix) == 0) suggestions.push_back(name);
        }
        std::sort(suggestions.begin(), suggestions.end());
        return suggestions;
    }

    void Console::autocomplete() {
        if (input_string.empty()) return;
        auto suggestions = get_suggestions(input_string);
        if (suggestions.size() == 1) {
            input_string = suggestions[0] + " ";
        } else if (suggestions.size() > 1) {
            std::string common_prefix = suggestions[0];
            for (size_t i = 1; i < suggestions.size(); ++i) {
                size_t j = 0;
                while (j < common_prefix.length() && j < suggestions[i].length() &&
                       common_prefix[j] == suggestions[i][j]) ++j;
                common_prefix = common_prefix.substr(0, j);
            }
            if (common_prefix.length() > input_string.length()) {
                input_string = common_prefix;
            }
        }
        cursor_position = input_string.length();
    }

    void Console::handle_event(const sf::Event& event, sf::RenderWindow& window) {
        if (event.is<sf::Event::KeyPressed>()) {
            const auto* key = event.getIf<sf::Event::KeyPressed>();
            if (!key) return;

            if (!visible) {
                if (key->code == sf::Keyboard::Key::Grave) {
                    show();
                    return;
                }
            }

            bool is_ctrl = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
            bool is_shift = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);

            cursor_blink_time = 0.f;
            cursor_visible = true;

            switch (key->code) {
                case sf::Keyboard::Key::Escape: hide(); break;
                case sf::Keyboard::Key::Enter:
                    if (text_selection.is_active()) remove_selection();
                    execute_command(input_string);
                    input_string.clear();
                    cursor_position = 0;
                    clear_selection();
                    break;
                case sf::Keyboard::Key::Backspace:
                    if (text_selection.is_active()) {
                        remove_selection();
                    } else if (cursor_position > 0 && !input_string.empty()) {
                        if (is_ctrl) {
                            int start = prev_word_position(cursor_position, input_string);
                            if (start < cursor_position) {
                                input_string.erase(start, cursor_position - start);
                                cursor_position = start;
                            }
                        } else {
                            input_string.erase(cursor_position - 1, 1);
                            cursor_position--;
                        }
                    }
                    break;
                case sf::Keyboard::Key::Delete:
                    if (text_selection.is_active()) {
                        remove_selection();
                    } else if (cursor_position < static_cast<int>(input_string.length())) {
                        input_string.erase(cursor_position, 1);
                    }
                    break;
                case sf::Keyboard::Key::Left: {
                    int new_pos = cursor_position;
                    if (is_ctrl) new_pos = prev_word_position(cursor_position, input_string);
                    else if (cursor_position > 0) new_pos = cursor_position - 1;
                    if (is_shift) {
                        if (!text_selection.is_active()) {
                            text_selection.start = cursor_position;
                            text_selection.end = cursor_position;
                        }
                        cursor_position = new_pos;
                        text_selection.end = cursor_position;
                    } else {
                        clear_selection();
                        cursor_position = new_pos;
                    }
                    break;
                }
                case sf::Keyboard::Key::Right: {
                    int new_pos = cursor_position;
                    if (is_ctrl) new_pos = next_word_position(cursor_position, input_string);
                    else if (cursor_position < static_cast<int>(input_string.length())) new_pos = cursor_position + 1;
                    if (is_shift) {
                        if (!text_selection.is_active()) {
                            text_selection.start = cursor_position;
                            text_selection.end = cursor_position;
                        }
                        cursor_position = new_pos;
                        text_selection.end = cursor_position;
                    } else {
                        clear_selection();
                        cursor_position = new_pos;
                    }
                    break;
                }
                case sf::Keyboard::Key::Home:
                    if (is_shift) {
                        if (!text_selection.is_active()) {
                            text_selection.start = cursor_position;
                            text_selection.end = cursor_position;
                        }
                        cursor_position = 0;
                        text_selection.end = cursor_position;
                    } else {
                        clear_selection();
                        cursor_position = 0;
                    }
                    break;
                case sf::Keyboard::Key::End:
                    if (is_shift) {
                        if (!text_selection.is_active()) {
                            text_selection.start = cursor_position;
                            text_selection.end = cursor_position;
                        }
                        cursor_position = static_cast<int>(input_string.length());
                        text_selection.end = cursor_position;
                    } else {
                        clear_selection();
                        cursor_position = static_cast<int>(input_string.length());
                    }
                    break;
                case sf::Keyboard::Key::Up:
                    if (!command_history.empty() && history_index < static_cast<int>(command_history.size()) - 1) {
                        history_index++;
                        input_string = command_history[command_history.size() - 1 - history_index];
                        cursor_position = input_string.length();
                    }
                    break;
                case sf::Keyboard::Key::Down:
                    if (history_index > 0) {
                        history_index--;
                        input_string = command_history[command_history.size() - 1 - history_index];
                        cursor_position = input_string.length();
                    } else if (history_index == 0) {
                        history_index = -1;
                        input_string.clear();
                        cursor_position = 0;
                    }
                    break;
                case sf::Keyboard::Key::Tab:
                    autocomplete();
                    break;
                case sf::Keyboard::Key::PageUp:
                    scroll_offset -= scroll_speed * 10;
                    clamp_scroll();
                    break;
                case sf::Keyboard::Key::PageDown:
                    scroll_offset += scroll_speed * 10;
                    clamp_scroll();
                    break;
                case sf::Keyboard::Key::C:
                    if (is_ctrl && text_selection.is_active()) {
                        int start = text_selection.get_start();
                        int end = text_selection.get_end();
                        sf::Clipboard::setString(input_string.substr(start, end - start));
                    }
                    break;
                case sf::Keyboard::Key::V:
                    if (is_ctrl) {
                        std::string clip = sf::Clipboard::getString();
                        if (!clip.empty()) {
                            if (text_selection.is_active()) remove_selection();
                            input_string.insert(cursor_position, clip);
                            cursor_position += static_cast<int>(clip.length());
                        }
                    }
                    break;
                case sf::Keyboard::Key::X:
                    if (is_ctrl && text_selection.is_active()) {
                        int start = text_selection.get_start();
                        int end = text_selection.get_end();
                        sf::Clipboard::setString(input_string.substr(start, end - start));
                        remove_selection();
                    }
                    break;
                case sf::Keyboard::Key::A:
                    if (is_ctrl) select_all();
                    break;
                default: break;
            }
        }

        if (visible) {
            if (event.is<sf::Event::TextEntered>()) {
                const auto* text = event.getIf<sf::Event::TextEntered>();
                if (text && text->unicode >= 32 && text->unicode < 127) {
                    if (text_selection.is_active()) remove_selection();
                    input_string.insert(cursor_position, 1, static_cast<char>(text->unicode));
                    cursor_position++;
                }
            }
            if (event.is<sf::Event::MouseWheelScrolled>()) {
                const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>();
                if (scroll) {
                    scroll_offset -= scroll->delta * scroll_speed;
                    clamp_scroll();
                }
            }
        }
    }

    void Console::rebuild_display_texts() {
        render_items.clear();
        if (!font_ptr) return;

        float max_width = console_width - 30.f;
        float x = 15.f;
        float y = 0.f;
        float line_height = static_cast<float>(char_size) * 1.2f;

        for (const auto& msg : messages) {
            for (const auto& frag : msg) {
                sf::Text txt(*font_ptr, frag.text, frag.character_size);
                txt.setFillColor(frag.color);
                unsigned int style = sf::Text::Regular;
                if (frag.bold) style |= sf::Text::Bold;
                if (frag.italic) style |= sf::Text::Italic;
                if (frag.underlined) style |= sf::Text::Underlined;
                txt.setStyle(style);

                float frag_width = txt.getLocalBounds().size.x;
                if (x + frag_width > max_width && x > 15.f) {
                    x = 15.f;
                    y += line_height;
                }
                txt.setPosition({x, y});
                render_items.push_back({std::move(txt), sf::Vector2f(x, y)});
                x += frag_width;
            }
            x = 15.f;
            y += line_height;
        }
        total_text_height = y + line_height;
        texts_dirty = false;
    }

    void Console::update(sf::RenderWindow& window, float dt) {
        if (!inited) return;

        if (!visible) return;
        update_cursor(dt);

        const auto window_size = static_cast<sf::Vector2f>(window.getSize());

        background_rect.setFillColor(background_color);
        background_rect.setOutlineThickness(theme_outline_thickness);
        background_rect.setOutlineColor(theme_outline_color);
        background_rect.setSize({console_width, console_height});
        background_rect.setPosition({0.f, 0.f});

        head_rect.setFillColor(foreground_color);
        head_rect.setOutlineThickness(theme_outline_thickness);
        head_rect.setOutlineColor(theme_outline_color);
        head_rect.setSize({console_width, 35.f});
        head_rect.setPosition({0.f, 0.f});

        head_title->setPosition({10.f, 5.f});

        auto title_position = head_title->getPosition();
        auto title_bounds = head_title->getLocalBounds();
        fps_text->setCharacterSize(char_size * 0.90f);
        fps_text->setPosition(title_position + sf::Vector2f{
            title_bounds.size.x + 5.f,
            title_bounds.size.y / 2.f
        });
        fps_text->setString(std::to_string(static_cast<int>(MainLoop::get_fps())) + " fps");

        input_rect.setFillColor(foreground_color);
        input_rect.setOutlineThickness(theme_outline_thickness);
        input_rect.setOutlineColor(theme_outline_color);
        input_rect_text->setString(input_string.empty() ? "Enter command..." : input_string);
        input_rect_text->setFillColor(input_string.empty() ? sf::Color(100, 100, 100, 150) : sf::Color(220, 220, 220));

        float text_height = input_rect_text->getLocalBounds().size.y;
        float input_height = text_height + input_padding * 2;
        input_height = std::max(input_height, 30.f);
        input_rect.setSize({window_size.x - 20.f, input_height});
        input_rect.setPosition({10.f, console_height - input_height - 10.f});

        input_rect_text->setCharacterSize(char_size);
        input_rect_text->setPosition({
            input_rect.getPosition().x + input_padding / 2.f,
            input_rect.getPosition().y + input_padding
        });

        cursor_rect.setSize({2.f, input_rect.getSize().y - 5.f});
        float cursor_x = input_rect_text->findCharacterPos(cursor_position).x;
        float cursor_y = input_rect.getPosition().y + (input_rect.getSize().y - cursor_rect.getSize().y) / 2.f;
        cursor_rect.setPosition({cursor_x, cursor_y});

        if (texts_dirty) {
            rebuild_display_texts();
        }

        float viewport_height = input_rect.getPosition().y - head_rect.getSize().y - 15.f;
        if (viewport_height <= 0.f) {
            max_scroll = 0.f;
        } else {
            max_scroll = std::max(0.f, total_text_height - viewport_height);
        }
        clamp_scroll();

        float headBottom = head_rect.getSize().y + 5.f;
        for (auto& item : render_items) {
            item.text.setPosition({item.position.x, headBottom + item.position.y - scroll_offset});
        }

        update_scrollbar(window);

        if (text_selection.is_active()) {
            int start = text_selection.get_start();
            int end = text_selection.get_end();
            sf::Vector2f left_pos  = input_rect_text->findCharacterPos(start);
            sf::Vector2f right_pos = input_rect_text->findCharacterPos(end);
            float y = input_rect.getPosition().y + 3.f;
            float height = input_rect.getSize().y - 6.f;
            selection_rect.setFillColor(selection_color);
            selection_rect.setPosition({left_pos.x, y});
            selection_rect.setSize({right_pos.x - left_pos.x, height});
        }

        suggestion_text->setString("");
        if (!input_string.empty()) {
            auto suggestions = get_suggestions(input_string);
            if (!suggestions.empty()) {
                std::string combined;
                if (suggestions.size() == 1) combined = suggestions[0];
                else {
                    for (auto s : suggestions) combined += s + "\n";
                }
                suggestion_text->setString(combined);
                sf::Vector2f suggSize = suggestion_text->getLocalBounds().size;
                suggestion_text->setPosition({
                    input_rect_text->getPosition().x,
                    input_rect.getPosition().y - suggSize.y - char_size
                });
                suggestion_rect.setSize({suggSize.x + 10.f, suggSize.y + 10.f});
                suggestion_rect.setPosition({
                    suggestion_text->getPosition().x - 5.f,
                    suggestion_text->getPosition().y - 5.f
                });
            }
        }
    }

    void Console::render(sf::RenderWindow& window) {
        if (!inited) return;
        if (!visible) return;

        window.draw(background_rect);
        window.draw(head_rect);
        window.draw(*head_title);
        window.draw(*fps_text);
        window.draw(input_rect);
        if (text_selection.is_active()) window.draw(selection_rect);
        window.draw(*input_rect_text);
        if (cursor_visible) window.draw(cursor_rect);

        sf::View original_view = window.getView();
        sf::Vector2f window_size = static_cast<sf::Vector2f>(window.getSize());
        float view_top = head_rect.getSize().y + 5.f;
        float view_height = input_rect.getPosition().y - head_rect.getSize().y - 15.f;
        if (view_height > 0.f) {
            sf::View console_view(sf::FloatRect(
                sf::Vector2f(0.f, view_top),
                sf::Vector2f(window_size.x, view_height)
            ));
            console_view.setViewport(sf::FloatRect(
                sf::Vector2f(0.f, view_top / window_size.y),
                sf::Vector2f(1.f, view_height / window_size.y)
            ));
            window.setView(console_view);
            for (const auto& item : render_items) {
                window.draw(item.text);
            }
        }
        window.setView(original_view);

        if (suggestion_text && !suggestion_text->getString().isEmpty()) {
            window.draw(suggestion_rect);
            window.draw(*suggestion_text);
        }
        if (max_scroll > 0.f) {
            window.draw(scrollbar_track);
            window.draw(scrollbar_thumb);
        }
    }

    void Console::update_scrollbar(sf::RenderWindow& window) {
        float viewport_height = input_rect.getPosition().y - head_rect.getSize().y - 15.f;
        sf::Vector2f window_size = static_cast<sf::Vector2f>(window.getSize());
        if (viewport_height <= 0.f || max_scroll <= 0.f) {
            scrollbar_track.setSize({0.f, 0.f});
            scrollbar_thumb.setSize({0.f, 0.f});
            return;
        }
        float track_height = viewport_height;
        float thumb_height = std::max(20.f, (viewport_height / (total_text_height + 1.f)) * track_height);
        float thumb_pos = (scroll_offset / max_scroll) * (track_height - thumb_height);

        scrollbar_track.setSize({8.f, track_height});
        scrollbar_track.setPosition({window_size.x - 18.f, head_rect.getSize().y + 5.f});

        scrollbar_thumb.setSize({8.f, thumb_height});
        scrollbar_thumb.setPosition({window_size.x - 18.f, scrollbar_track.getPosition().y + thumb_pos});
    }

    void Console::update_cursor(float dt) {
        cursor_blink_time += dt;
        if (cursor_blink_time >= 0.5f) {
            cursor_blink_time = 0.f;
            cursor_visible = !cursor_visible;
        }
    }

    void Console::clamp_scroll() {
        scroll_offset = std::max(0.f, std::min(scroll_offset, max_scroll));
    }

    void Console::clear_selection() {
        text_selection.reset();
    }

    void Console::remove_selection() {
        if (!text_selection.is_active()) return;
        int start = text_selection.get_start();
        int end = text_selection.get_end();
        input_string.erase(start, end - start);
        cursor_position = start;
        clear_selection();
    }

    void Console::select_all() {
        text_selection.start = 0;
        text_selection.end = static_cast<int>(input_string.length());
        cursor_position = text_selection.end;
    }

    void Console::add_to_history(const std::string& command) {
        if (!command.empty()) {
            command_history.push_back(command);
            if (command_history.size() > max_history) command_history.pop_front();
        }
        history_index = -1;
        save_history();
    }

    void Console::register_default_commands() {
        register_command(
            "help",
            [this](const std::vector<std::string>& args) {
                if (args.empty()) {
                    print_plain("Available Commands: ", sf::Color::Cyan);
                    for (const auto& [name, cmd] : commands) {
                        print_plain("  " + name + " - " + cmd.description);
                    }
                } else {
                    auto it = commands.find(args[0]);
                    if (it != commands.end()) {
                        print_plain("Command: " + it->second.name);
                        print_plain("Description: " + it->second.description);
                        print_plain("Usage: " + it->second.usage);
                    } else {
                        print_error("Unknown command: " + args[0]);
                    }
                }
            },
            "Get help",
            "help <command>"
        );

        register_command(
            "clear",
            [this](const std::vector<std::string>&) {
                messages.clear();
                render_items.clear();
                total_text_height = 0.f;
                texts_dirty = true;
            },
            "Clear console messages",
            "clear"
        );

        register_command(
            "console.char_size",
            [this](const std::vector<std::string>& args) {
                if (!args.empty()) {
                    try {
                        int cs = std::stoi(args[0]);
                        char_size = cs;
                        config_file->set("char_size", char_size);
                        texts_dirty = true;
                    } catch (const std::exception& e) {
                        print_error(e.what());
                    }
                }
            },
            "Set console font character size",
            "console.char_size <value>"
        );

        register_command(
            "console.theme_outline_thickness",
            [this](const std::vector<std::string>& args) {
                if (!args.empty()) {
                    try {
                        float tot = std::stof(args[0]);
                        theme_outline_thickness = tot;
                        config_file->set("theme_outline_thickness", tot);
                    } catch (const std::exception& e) {
                        print_error(e.what());
                    }
                }
            },
            "Set console theme outline thickness",
            "console.theme_outline_thickness <value>"
        );

        register_command(
            "console.theme_outline_color",
            [this](const std::vector<std::string>& args) {
                if (!args.empty()) {
                    try {
                        theme_outline_color = me::parsing::str_to_color(args);
                        config_file->set("theme_outline_color", me::parsing::color_to_str(theme_outline_color));
                    } catch (const std::exception& e) {
                        print_error(e.what());
                    }
                }
            },
            "Set console theme outline color",
            "console.theme_outline_color <r g b a>"
        );

        register_command(
            "console.foreground_color",
            [this](const std::vector<std::string>& args) {
                if (!args.empty()) {
                    try {
                        foreground_color = me::parsing::str_to_color(args);
                        config_file->set("foreground_color", me::parsing::color_to_str(foreground_color));
                    } catch (const std::exception& e) {
                        print_error(e.what());
                    }
                }
            },
            "Set console foreground color",
            "console.foreground_color <r g b a>"
        );

        register_command(
            "console.background_color",
            [this](const std::vector<std::string>& args) {
                if (!args.empty()) {
                    try {
                        background_color = me::parsing::str_to_color(args);
                        config_file->set("background_color", me::parsing::color_to_str(background_color));
                    } catch (const std::exception& e) {
                        print_error(e.what());
                    }
                }
            },
            "Set console background color",
            "console.background_color <r g b a>"
        );

        register_command(
            "console.height",
            [this](const std::vector<std::string>& args) {
                if (!args.empty()) {
                    try {
                        int ch = std::stoi(args[0]);
                        console_height = ch;
                        config_file->set("console_height", console_height);
                    } catch (const std::exception& e) {
                        print_error(e.what());
                    }
                }
            },
            "Set console height",
            "console.height <value>"
        );

        register_command(
            "console.width",
            [this](const std::vector<std::string>& args) {
                if (!args.empty()) {
                    try {
                        int ch = std::stoi(args[0]);
                        console_width = ch;
                        config_file->set("console_width", console_width);
                        texts_dirty = true;
                    } catch (const std::exception& e) {
                        print_error(e.what());
                    }
                }
            },
            "Set console width",
            "console.width <value>"
        );
    }

} // namespace me