#pragma once
#include <entt/entity/entity.hpp>

namespace me::hierarchy::cmp {
    struct Children { std::vector<entt::entity> entities; };
}