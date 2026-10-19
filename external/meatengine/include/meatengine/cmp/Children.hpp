#pragma once
#include <entt/entity/entity.hpp>

namespace me::cmp {
    struct Children { std::vector<entt::entity> entities; };
}