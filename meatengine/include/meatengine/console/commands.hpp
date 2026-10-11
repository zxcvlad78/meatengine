#pragma once

#include <entt/entt.hpp>
#include <SFML/Audio.hpp>
#include <meatengine/meatengine.hpp>


namespace me::console_commands {
    inline void init() {
        Console::get_instance().register_command(
            "exit",
            [](const std::vector<std::string>& args) {
                me::MainLoop::get_window().close();
            },
            "Close window and exit",
            "exit"
        );
        Console::get_instance().register_command(
            "echo.mode",
            [](const std::vector<std::string>& args) {
                if (args.empty()) {
                    Console::get_instance().print("echo mode: " + std::to_string(Console::get_instance().echo_mode));
                    return;
                }
                try {
                    Console::get_instance().echo_mode = std::stoi(args[0]);

                } catch (const std::exception& e) {
                    Console::get_instance().print_error(e.what());
                }
            },
            "Echo mode",
            "echo, echo <bool>"
        );
        Console::get_instance().register_command(
            "echo",
            [](const std::vector<std::string>& args) {
                std::string total_string;
                for (auto c : args) {
                    total_string += c + " ";
                }

                try {
                    Console::get_instance().print(total_string);
                } catch (const std::exception& e) {
                    Console::get_instance().print_error(e.what());
                }
            },
            "Echo message",
            "echo <string>"
        );
        Console::get_instance().register_command(
            "cfg.save",
            [](const std::vector<std::string>& args) {
                Console::get_instance().config_file->save();
            },
            "Save cfg file",
            "cfg.save"
        );
        Console::get_instance().register_command(
            "cfg.load",
            [](const std::vector<std::string>& args) {
                Console::get_instance().load_cfg(me::MainLoop::get_window());
            },
            "Load cfg file",
            "cfg.load"
        );
        Console::get_instance().register_command(
            "cfg.reset",
            [](const std::vector<std::string>& args) {
                Console::get_instance().reset_cfg(me::MainLoop::get_window());
            },
            "Reset cfg file",
            "cfg.reset"
        );
        Console::get_instance().register_command(
            "fps.max",
            [](const std::vector<std::string>& args) {
                if (!args.empty()) {
                    try {
                        int fps = std::stoi(args[0]);
                        me::MainLoop::set_framerate_limit(fps);
                    } catch (const std::exception& e) {
                        Console::get_instance().print_error(e.what());
                    }
                }
            },
            "Set target framerate",
            "fps.max <int>"
        );
        Console::get_instance().register_command(
            "volume",
            [](const std::vector<std::string>& args) {
                if (!args.empty()) {
                    try {
                        //float vol = std::stoi(args[0]);
                        sf::Listener::setGlobalVolume(std::stoi(args[0]));
                    } catch (const std::exception& e) {
                        Console::get_instance().print_error(e.what());
                    }
                }
            },
            "Set audio volume (0.0-100.0)",
            "volume <float>"
        );
        // Console::get_instance().register_command(
        //     "speed",
        //     [](const std::vector<std::string>& args) {
        //         if (!args.empty()) {
        //             try {
        //                 float val = std::stof(args[0]);
        //                 if (val >= 0.f) {
        //                     me::MainLoop::dt_scale = val;
        //                     Console::get_instance().print_success("Speed scale set to: " + std::to_string(val));
        //                 } else {
        //                     Console::get_instance().print_error("Speed scale must be positive");
        //                 }
        //             } catch (const std::exception& e) {
        //                 Console::get_instance().print_error(e.what());
        //             }
        //         } else {
        //             Console::get_instance().print_success("Current speed scale: " + std::to_string(me::MainLoop::dt_scale));
        //         }
        //     },
        //     "Set time speed multiplier",
        //     "speed <float>"
        // );
        Console::get_instance().register_command(
            "lua.run",
            [](const std::vector<std::string>& args) {
                //if (args.size() != 1) return;
                try {
                    const std::string& text = args.at(0);
                    if (text.starts_with("path::")) {
                        std::string path = text;
                        path.erase(0, 6);
                        me::ScriptingServer::run_file(path);
                    }
                    else {
                        std::string total_string;
                        for(std::string s : args) total_string.append(s + " ");
                        //total_string.erase(-1);

                        me::ScriptingServer::run_string(total_string);
                    }
                } catch (const std::exception& e) {
                    Console::get_instance().print_error(e.what());
                }
            },
            "Run lua script/string",
            "(run script) lua.run <path::path/to/script.lua>\n(run string) lua.run <script string>"
        );


        // Test
        // for (uint8_t i = 0; i < 25; i ++) {
        //     std::string str_i = std::to_string(i);
        //     Console::get_instance().register_command(
        //         "sas" + str_i,
        //         [](const std::vector<std::string>& args) { },
        //         "pro100 sas nomer " + str_i,
        //         "nikak))"
        //     );
        // }
    }
}