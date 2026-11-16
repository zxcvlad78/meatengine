#pragma once
#include <string>
#include <functional>

class App;

namespace me::app {
class Plugin {
public:
    virtual ~Plugin() = default;
    virtual void build(App& app) = 0;
    virtual bool ready(App& app) { return true; }
    virtual void finish(App& app) {}
    virtual void cleanup(App& app) {}
    virtual const char* name() const = 0;
    virtual bool is_unique() const { return true; }
};
}