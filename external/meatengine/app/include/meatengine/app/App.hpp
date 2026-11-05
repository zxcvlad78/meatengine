#pragma once
#include <string>

#include <meatengine/binres/mainfont.h>
#include <meatengine/MainLoop.hpp>
#include <meatengine/ModLoader.hpp>
#include <meatengine/ResourceLoader.hpp>
#include <meatengine/Resources.hpp>

namespace me {
    class App {
    private:
        std::string name_;

    public:
        App(const std::string& name)
        : name_(name) {
            auto font = std::make_shared<me::Font>();  
            try {
                auto success = font->res.openFromMemory(______data_core_res_fonts_mainfont_ttf, ______data_core_res_fonts_mainfont_ttf_len);
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

        inline void set_name(const std::string& name) { name_ = std::move(name); }
        inline const std::string& get_name() { return name_; }

        inline App& run() {
            me::MainLoop::init(name_);
            me::ModLoader::load("data");

            return *this;
        }
    };
} // namespace me
