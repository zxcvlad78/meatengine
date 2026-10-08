#include "meatengine/lua_bindings/ComponentDescriptor.hpp"

#include <algorithm>
#include <stdexcept>

namespace me::lua_bindings {

std::unordered_map<std::string, ComponentDescriptor>& descriptors() {
    static std::unordered_map<std::string, ComponentDescriptor> d;
    return d;
}

std::vector<ViewCache>& view_cache() {
    static std::vector<ViewCache> c;
    return c;
}

void reset_view_cache() {
    view_cache().clear();
}

}