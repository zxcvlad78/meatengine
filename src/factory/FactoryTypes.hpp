#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace godlike::factory {

using ItemId = std::uint32_t;
using ProtoId = std::uint32_t;
using TileId = std::uint32_t;

constexpr ItemId kNoItem = 0;

enum class ProtoType {
    Unknown,
    Item,
    Recipe,
    Technology,
    Entity,
    Tile,
};

enum class Capability {
    None,
    Mining,
    Crafting,
    Storage,
    Logistic,
    Inserter,
    Research,
};

const char* capability_name(Capability capability);
Capability capability_from_name(const std::string& name);

struct ItemProto {
    std::string name;
    std::string label;
    ItemId id = kNoItem;
    int stack_size = 100;
    std::string icon;
    std::string research_unit;
};

struct RecipeProto {
    std::string name;
    std::string label;
    std::vector<std::pair<ItemId, int>> ingredients;
    ItemId result = kNoItem;
    int result_count = 1;
    float time = 1.0f;
    std::string category;
    bool hidden = false;
};

struct TechnologyProto {
    std::string name;
    std::string label;
    std::vector<std::string> prerequisites;
    float unit_count = 10.0f;
    std::vector<std::pair<std::string, float>> effects;
};

struct EntityProto {
    std::string name;
    std::string label;
    std::vector<Capability> capabilities;
    std::vector<std::string> recipes;
    std::vector<std::string> filters;
    std::vector<std::pair<ItemId, int>> cost;
    int width = 1;
    int height = 1;
    int input_slots = 0;
    int output_slots = 0;
    int storage_slots = 0;
    float speed = 1.0f;
    std::string graphics;
    std::vector<std::string> overlays;
    float anchor_x = -32.0f;
    float anchor_y = -64.0f;
    float overlay_y = 0.0f;
    bool flat = false;
    bool directional = false;
    bool placeable = true;
    std::uint32_t filter_color = 0;
};

struct TileProto {
    std::string name;
    std::string label;
    TileId id = 0;
    std::string ore_item;
    std::string depleted_tile;
    int ore_amount = 0;
    bool minable = false;
    bool buildable = true;
    std::string graphics;
};

struct Inventory {
    std::vector<std::pair<ItemId, int>> slots;

    int count(ItemId item) const;
    bool has(ItemId item, int amount) const;
    int add(ItemId item, int amount);
    bool remove(ItemId item, int amount);
    bool empty() const { return slots.empty(); }
    void clear() { slots.clear(); }
    int size() const { return static_cast<int>(slots.size()); }
};

struct Entity {
    ProtoId proto = 0;
    int x = 0;
    int y = 0;
    int dir = 2;
    int recipe = -1;
    float progress = 0.0f;
    float swing = 0.0f;
    ItemId held = kNoItem;
    Inventory input;
    Inventory output;

    std::uint64_t key() const {
        return static_cast<std::uint64_t>(y) * 1024ull + static_cast<std::uint64_t>(x);
    }
};

struct PlayerState {
    Inventory inventory;
    std::string cursor_item;
    int cursor_count = 0;
};

}
