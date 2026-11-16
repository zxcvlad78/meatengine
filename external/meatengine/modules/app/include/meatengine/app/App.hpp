#pragma once
#include <string>
#include <vector>

#include <meatengine/assets/binary/mainfont.h>
#include <meatengine/assets/ResourceLoader.hpp>
#include <meatengine/assets/Resources.hpp>
#include <meatengine/app/MainLoop.hpp>
#include <meatengine/app/Plugin.hpp>

namespace me {
    class App {
    private:
        std::string name_;
        std::vector<app::Plugin> plugins_;
    public:
        App(const std::string& name)
        : name_(name) {
            auto font = std::make_shared<me::Font>();  
            try {
                auto success = font->res.openFromMemory(______mainfont_ttf, ______mainfont_ttf_len);
                if (success) {
                    entt::resource<me::Font> main_font{std::move(font)};
                    me::ResourceLoader::set_default<me::Font>(std::move(main_font));
                } else {
                    std::cout << "[App] failed to open main font from memory\n";
                }
            } catch(const std::exception& e) {
                std::cout << "[App] " << e.what() << std::endl;
            }
        }

        inline App& set_name(const std::string& name) {
            name_ = std::move(name);
            return *this;
        }
        [[nodiscard]] inline const std::string& get_name() { return name_; }

        inline App& run() {
            me::MainLoop::init(name_);
            return *this;
        }

        template<typename T, typename... Args>
        App& add_plugins(Args&&... args) {
            auto plugin = std::make_unique<T>(std::forward<Args>(args)...);
            plugin->build(*this);
            plugins_.push_back(std::move(plugin));
            return *this;
        }
    };
} // namespace me
