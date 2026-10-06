#pragma once

#include <godlike/ItemDefinition.hpp>
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <stdexcept>

namespace godlike {

class ItemRegistry {
public:
    ItemRegistry() = delete;

    static uint16_t register_def(
        const std::string& id,
        uint32_t max_stack,
        std::string icon_path
    ) {
        if (id.empty())
            throw std::runtime_error("[ItemRegistry] empty item id");
        if (max_stack == 0)
            throw std::runtime_error("[ItemRegistry] max_stack=0 for '" + id + "'");

        auto it = _id_map.find(id);
        if (it != _id_map.end()) {
            uint16_t idx = it->second;
            _definitions[idx] = ItemDefinition{
                id, std::move(icon_path), max_stack
            };
            return idx;
        }

        uint16_t idx = static_cast<uint16_t>(_definitions.size());
        _definitions.push_back(ItemDefinition{
            id, std::move(icon_path), max_stack
        });
        _id_map.emplace(id, idx);
        return idx;
    }

    static const ItemDefinition& get(uint16_t idx) {
        if (idx >= _definitions.size())
            throw std::runtime_error("[ItemRegistry] invalid item index");
        return _definitions[idx];
    }

    static uint16_t find(const std::string& id) {
        auto it = _id_map.find(id);
        if (it == _id_map.end()) return 0;
        return it->second;
    }

    static const std::string& id_of(uint16_t idx) { return get(idx).id; }
    static std::size_t size() noexcept { return _definitions.size(); }

private:
    inline static std::vector<ItemDefinition> _definitions = { ItemDefinition{"invalid_item"} };
    inline static std::unordered_map<std::string, uint16_t> _id_map;
};

} // namespace godlike