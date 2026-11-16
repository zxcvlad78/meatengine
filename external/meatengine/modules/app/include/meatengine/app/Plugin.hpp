#pragma once
#include <string>
#include <functional>


namespace me {    
class App;
namespace app {
class Plugin {
public:
    virtual ~Plugin() = default;
    virtual void build(App&) =     0;
    virtual bool ready(App&)       { return true; }
    virtual void finish(App&)      {}
    virtual void cleanup(App&)     {}
    virtual const char* name() const = 0;
    virtual bool is_unique() const     { return true; }
};
}
}