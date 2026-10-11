#pragma once

#include <entt/entt.hpp>

#include <functional>
#include <string>
#include <vector>

namespace me::registry_setup {

using SetupFunc = std::function<void(entt::registry&)>;
struct SetupEntry { std::string name; SetupFunc func; };

std::vector<SetupEntry>& list();

void run(entt::registry& reg);

struct SetupAutoReg {
    SetupAutoReg(std::string name, SetupFunc fn) {
        list().push_back({ std::move(name), std::move(fn) });
    }
};

} // namespace me

#define ME_REGISTER_REGISTRY_SETUP_IMPL(NAME, FN, CNT) \
    namespace { \
        const me::registry_setup::SetupAutoReg _me_reg_setup_##CNT{ NAME, (FN) }; \
    }

#define ME_REGISTER_REGISTRY_SETUP(NAME, FN) \
    ME_REGISTER_REGISTRY_SETUP_IMPL(NAME, FN, __COUNTER__)