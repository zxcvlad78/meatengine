#include <meatengine/td/three_d.hpp>

#include <glad/glad.h>

#include <iostream>

#include <meatengine/app/MainLoop.hpp>

namespace me::td {

    namespace {
        bool g_gl_initialized = false;

        // Static init: run the GL loader once the window (and with it the GL
        // context) exists. The module must be linked with WHOLE_ARCHIVE (the
        // default in meatengine builds) for this to survive.
        const struct GlInitHook {
            GlInitHook() {
                me::MainLoop::add_on_init([](sf::RenderWindow&) {
                    init_gl();
                });
            }
        } g_gl_init_hook;
    }

    void init_gl() {
        if (g_gl_initialized) return;
        if (!gladLoadGL()) {
            std::cerr << "[td] failed to initialize the OpenGL loader\n";
            return;
        }
        g_gl_initialized = true;

        GLint major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        std::cerr << "[td] OpenGL " << major << '.' << minor << " initialized\n";
    }
}
