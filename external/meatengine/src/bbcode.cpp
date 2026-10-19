#include <meatengine/bbcode.hpp>
#include <meatengine/parsing.hpp>

namespace me::BBCode {
    std::vector<TextFragment> parse(const std::string& input, unsigned int default_size) {
        std::vector<TextFragment> result;

        struct StyleState {
            sf::Color color;
            unsigned int size;
            bool bold, italic, underlined;
        };

        std::vector<StyleState> style_state;
        StyleState current;
        current.color = sf::Color::White;
        current.size = default_size;
        current.bold = false;
        current.italic = false;
        current.underlined = false;

        std::string buffer;
        size_t pos = 0;
        while (pos < input.length()) {
            if (input[pos] == '[') {
                if (pos + 1 < input.length() && input[pos + 1] == '[') {
                    buffer += '[';
                    pos += 2;
                    continue;
                }
                size_t end = input.find(']', pos);
                if (end == std::string::npos) {
                    buffer += input[pos++];
                    continue;
                }
                std::string tag = input.substr(pos + 1, end - pos - 1);
                pos = end + 1;

                if (!tag.empty() && tag[0] == '/') {
                    std::string closeTag = tag.substr(1);
                    if (!buffer.empty()) {
                        result.emplace_back(
                            buffer, current.color, current.size,
                            current.bold, current.italic, current.underlined
                        );
                        buffer.clear();
                    }
                    if (!style_state.empty()) {
                        current = style_state.back();
                        style_state.pop_back();
                    } else {
                        current.color = sf::Color::White;
                        current.size = default_size;
                        current.bold = false;
                        current.italic = false;
                        current.underlined = false;
                    }
                    continue;
                }

                style_state.push_back(current);
                if (!buffer.empty()) {
                    result.emplace_back(
                        buffer, current.color, current.size,
                        current.bold, current.italic, current.underlined
                    );
                    buffer.clear();
                }

                if (tag == "b") {
                    current.bold = true;
                } else if (tag == "i") {
                    current.italic = true;
                } else if (tag == "u") {
                    current.underlined = true;
                } else if (tag.rfind("color=", 0) == 0) {
                    std::string color_str = tag.substr(6);
                    current.color = me::parsing::str_to_color(color_str);
                } else if (tag.rfind("size=", 0) == 0) {
                    try {
                        int sz = std::stoi(tag.substr(5));
                        if (sz > 0) current.size = static_cast<unsigned int>(sz);
                    } catch (...) {}
                } else {
                    style_state.pop_back();
                }
                continue;
            }
            buffer += input[pos++];
        }
        if (!buffer.empty()) {
            result.emplace_back(
                buffer, current.color, current.size,
                current.bold, current.italic, current.underlined
            );
        }
        return result;
    }
} // namespace me::BBCode
