#pragma once
#include <meatengine/app/Plugin.hpp>
#include <meatengine/app/App.hpp>
#include <meatengine/ecs/SystemRegistry.hpp>
#include <entt/entt.hpp>

namespace me {
class TextEditorPlugin : public app::Plugin {
public:
    const char* name() const override { return "TimePlugin"; }

    void build(App& app) override {
        SystemRegistry.
    }
};
}