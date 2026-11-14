#pragma once

#include "FactoryTypes.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace godlike::factory {

struct MapSurface {
    std::string tile;
    float weight = 1.0f;
};

struct MapFeature {
    std::string tile;
    int count = 1;
    int radius = 2;
    float chance = 1.0f;
    int amount_min = 0;
    int amount_max = 0;
};

struct MapOre {
    std::string item;
    std::string tile;
    int count = 1;
    int radius = 2;
    int amount_min = 240;
    int amount_max = 720;
    bool near_spawn = false;
};

struct MapProto {
    std::string name;
    int width = 120;
    int height = 120;
    std::vector<MapSurface> surface;
    std::vector<MapFeature> features;
    std::vector<MapOre> ores;
    int spawn_clear_radius = 4;
    std::vector<std::pair<std::string, int>> starter_inventory;
};

class Registry {
public:
    static Registry& instance();

    void clear();

    ItemId register_item(const ItemProto& proto);
    ProtoId register_recipe(const RecipeProto& proto);
    ProtoId register_technology(const TechnologyProto& proto);
    ProtoId register_entity(const EntityProto& proto);
    TileId register_tile(const TileProto& proto);
    void register_map(const MapProto& proto);

    ItemId item_id(const std::string& name) const;
    ProtoId recipe_id(const std::string& name) const;
    ProtoId technology_id(const std::string& name) const;
    ProtoId entity_id(const std::string& name) const;
    TileId tile_id(const std::string& name) const;
    const MapProto* map() const;

    const ItemProto* item(ItemId id) const;
    const RecipeProto* recipe(ProtoId id) const;
    const TechnologyProto* technology(ProtoId id) const;
    const EntityProto* entity(ProtoId id) const;
    const TileProto* tile(TileId id) const;

    const std::vector<ItemProto>& items() const { return m_items; }
    const std::vector<RecipeProto>& recipes() const { return m_recipes; }
    const std::vector<TechnologyProto>& technologies() const { return m_technologies; }
    const std::vector<EntityProto>& entities() const { return m_entities; }
    const std::vector<TileProto>& tiles() const { return m_tiles; }

    ItemId add_item(const std::string& name);
    ProtoId add_recipe(const std::string& name);
    ProtoId add_entity(const std::string& name);
    TileId add_tile(const std::string& name);

private:
    std::vector<ItemProto> m_items;
    std::vector<RecipeProto> m_recipes;
    std::vector<TechnologyProto> m_technologies;
    std::vector<EntityProto> m_entities;
    std::vector<TileProto> m_tiles;
    std::unordered_map<std::string, ItemId> m_item_ids;
    std::unordered_map<std::string, ProtoId> m_recipe_ids;
    std::unordered_map<std::string, ProtoId> m_technology_ids;
    std::unordered_map<std::string, ProtoId> m_entity_ids;
    std::unordered_map<std::string, TileId> m_tile_ids;
    MapProto m_map;
    bool m_has_map = false;
};

class World {
public:
    static constexpr int kMaxWidth = 512;
    static constexpr int kMaxHeight = 512;

    void clear();
    void generate(std::uint32_t seed);

    bool ready() const { return m_ready; }
    std::uint32_t seed() const { return m_seed; }
    int width() const { return m_width; }
    int height() const { return m_height; }

    TileId tile_at(int x, int y) const;
    int tile_ore(int x, int y) const;
    void set_tile(int x, int y, TileId tile);
    bool in_map(int x, int y) const;

    Entity* entity_at(int x, int y);
    const Entity* entity_at(int x, int y) const;
    const std::unordered_map<std::uint64_t, Entity>& entities() const { return m_entities; }

    bool build(std::uint32_t peer, int x, int y, ProtoId proto, int dir, std::string* error);
    bool remove(int x, int y, std::string* error);
    bool mine(int x, int y, std::string* error);
    bool rotate(int x, int y, std::string* error);
    bool set_recipe(int x, int y, ProtoId recipe, std::string* error);

    void tick(float dt);
    void client_tick(float dt);

    bool transfer_item(int from_x, int from_y, int to_x, int to_y, ItemId item);
    bool accept_item(Entity& entity, ItemId item) const;

    void init_player(std::uint32_t peer);
    PlayerState& player(std::uint32_t peer) { return m_players[peer]; }
    const PlayerState& player(std::uint32_t peer) const;

    std::unordered_map<std::uint32_t, PlayerState>& players() { return m_players; }
    const std::unordered_map<std::uint32_t, PlayerState>& players() const { return m_players; }

    float research_units() const { return m_research_units; }
    int research_level() const { return m_research_level; }
    ProtoId active_technology() const { return m_active_technology; }
    const std::vector<ProtoId>& researched() const { return m_researched; }
    float modifier(const std::string& name) const;

    void add_research(float units);
    bool craft(std::uint32_t peer, ProtoId recipe, std::string* error);
    bool give_item(std::uint32_t peer, ItemId item, int count);
    bool take_item(std::uint32_t peer, ItemId item, int count);

    std::string encode_entities() const;
    std::string encode_inventory(std::uint32_t peer) const;
    std::string encode_research() const;
    void decode_entities(const std::string& data);
    void decode_inventory(std::uint32_t peer, const std::string& data);
    void decode_research(const std::string& data);

    void mark_dirty() { m_dirty = true; }
    bool dirty() const { return m_dirty; }
    void clear_dirty() { m_dirty = false; }
    std::uint64_t revision() const { return m_revision; }
    std::uint64_t crafted() const { return m_crafted; }
    std::uint64_t mined() const { return m_mined; }

private:
    void tick_mining(Entity& entity, float dt);
    void tick_crafting(Entity& entity, float dt);
    void tick_logistic(Entity& entity, float dt);
    void tick_inserter(Entity& entity, float dt);
    void tick_research(Entity& entity, float dt);
    void push_output(Entity& entity, int x, int y);
    void apply_effects(const TechnologyProto& proto);
    ProtoId next_technology() const;

    std::uint32_t m_seed = 0;
    bool m_ready = false;
    int m_width = 0;
    int m_height = 0;
    std::vector<TileId> m_tiles;
    std::vector<int> m_ore;
    std::unordered_map<std::uint64_t, Entity> m_entities;
    std::unordered_map<std::uint32_t, PlayerState> m_players;
    float m_research_units = 0.0f;
    int m_research_level = 0;
    ProtoId m_active_technology = 0;
    std::vector<ProtoId> m_researched;
    std::unordered_map<std::string, float> m_modifiers;
    std::uint64_t m_revision = 0;
    std::uint64_t m_crafted = 0;
    std::uint64_t m_mined = 0;
    std::uint64_t m_produced = 0;
    bool m_dirty = false;
};

}
