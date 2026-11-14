#include "FactoryWorld.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>

namespace godlike::factory {

namespace {

struct Rng {
    std::uint32_t state;

    explicit Rng(std::uint32_t seed) : state(seed ? seed : 0x9E3779B9u) {}

    std::uint32_t next() {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }

    int range(int lo, int hi) {
        if (hi <= lo) return lo;
        return lo + static_cast<int>(next() % static_cast<std::uint32_t>(hi - lo + 1));
    }

    float unit() { return static_cast<float>(next() % 100000u) / 100000.0f; }
};

std::vector<std::string> split(const std::string& text, char sep) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : text) {
        if (c == sep) {
            if (!current.empty()) parts.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    if (!current.empty()) parts.push_back(current);
    return parts;
}

std::vector<std::string> split_fields(const std::string& text, char sep) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : text) {
        if (c == sep) {
            parts.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    parts.push_back(current);
    return parts;
}

std::string join(const std::vector<std::string>& parts, char sep) {
    std::string out;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i) out.push_back(sep);
        out += parts[i];
    }
    return out;
}

std::string encode_items(const Inventory& inventory, const Registry& registry) {
    std::vector<std::string> parts;
    for (const auto& [item, count] : inventory.slots) {
        if (count <= 0 || item == kNoItem) continue;
        const ItemProto* proto = registry.item(item);
        parts.push_back((proto ? proto->name : std::string("?")) + ":" + std::to_string(count));
    }
    return join(parts, ',');
}

void decode_items(const std::string& text, Inventory& inventory, const Registry& registry) {
    inventory.clear();
    for (const std::string& record : split(text, ',')) {
        const std::size_t colon = record.rfind(':');
        if (colon == std::string::npos) continue;
        const ItemId item = registry.item_id(record.substr(0, colon));
        const int count = std::atoi(record.c_str() + colon + 1);
        if (item == kNoItem || count <= 0) continue;
        inventory.add(item, count);
    }
}

}

const char* capability_name(Capability capability) {
    switch (capability) {
        case Capability::Mining: return "mining";
        case Capability::Crafting: return "crafting";
        case Capability::Storage: return "storage";
        case Capability::Logistic: return "logistic";
        case Capability::Inserter: return "inserter";
        case Capability::Research: return "research";
        default: return "none";
    }
}

Capability capability_from_name(const std::string& name) {
    if (name == "mining") return Capability::Mining;
    if (name == "crafting") return Capability::Crafting;
    if (name == "storage") return Capability::Storage;
    if (name == "logistic") return Capability::Logistic;
    if (name == "inserter") return Capability::Inserter;
    if (name == "research") return Capability::Research;
    return Capability::None;
}

int Inventory::count(ItemId item) const {
    for (const auto& [slot, count] : slots)
        if (slot == item) return count;
    return 0;
}

bool Inventory::has(ItemId item, int amount) const {
    return count(item) >= amount;
}

int Inventory::add(ItemId item, int amount) {
    if (item == kNoItem || amount <= 0) return 0;
    int added = 0;
    for (auto& [slot, count] : slots) {
        if (slot != item) continue;
        count += amount;
        return amount;
    }
    slots.emplace_back(item, amount);
    return amount;
}

bool Inventory::remove(ItemId item, int amount) {
    if (amount <= 0) return true;
    for (auto& [slot, count] : slots) {
        if (slot != item) continue;
        count -= amount;
        if (count <= 0) {
            for (auto it = slots.begin(); it != slots.end(); ++it) {
                if (it->first == item) {
                    slots.erase(it);
                    break;
                }
            }
        }
        return true;
    }
    return false;
}

Registry& Registry::instance() {
    static Registry registry;
    return registry;
}

void Registry::clear() {
    m_items.clear();
    m_recipes.clear();
    m_technologies.clear();
    m_entities.clear();
    m_tiles.clear();
    m_item_ids.clear();
    m_recipe_ids.clear();
    m_technology_ids.clear();
    m_entity_ids.clear();
    m_tile_ids.clear();
    m_map = MapProto{};
    m_has_map = false;
}

ItemId Registry::register_item(const ItemProto& proto) {
    auto it = m_item_ids.find(proto.name);
    if (it != m_item_ids.end()) {
        m_items[it->second] = proto;
        return it->second;
    }
    ItemProto copy = proto;
    copy.id = static_cast<ItemId>(m_items.size() + 1);
    m_items.push_back(copy);
    const ItemId id = m_items.back().id;
    m_item_ids[proto.name] = id;
    return id;
}

ProtoId Registry::register_recipe(const RecipeProto& proto) {
    auto it = m_recipe_ids.find(proto.name);
    if (it != m_recipe_ids.end()) {
        m_recipes[it->second] = proto;
        return it->second;
    }
    m_recipes.push_back(proto);
    const ProtoId id = static_cast<ProtoId>(m_recipes.size() - 1);
    m_recipe_ids[proto.name] = id;
    return id;
}

ProtoId Registry::register_technology(const TechnologyProto& proto) {
    auto it = m_technology_ids.find(proto.name);
    if (it != m_technology_ids.end()) {
        m_technologies[it->second] = proto;
        return it->second;
    }
    m_technologies.push_back(proto);
    const ProtoId id = static_cast<ProtoId>(m_technologies.size() - 1);
    m_technology_ids[proto.name] = id;
    return id;
}

ProtoId Registry::register_entity(const EntityProto& proto) {
    auto it = m_entity_ids.find(proto.name);
    if (it != m_entity_ids.end()) {
        m_entities[it->second] = proto;
        return it->second;
    }
    m_entities.push_back(proto);
    const ProtoId id = static_cast<ProtoId>(m_entities.size() - 1);
    m_entity_ids[proto.name] = id;
    return id;
}

TileId Registry::register_tile(const TileProto& proto) {
    auto it = m_tile_ids.find(proto.name);
    if (it != m_tile_ids.end()) {
        m_tiles[it->second] = proto;
        return it->second;
    }
    m_tiles.push_back(proto);
    const TileId id = static_cast<TileId>(m_tiles.size() - 1);
    m_tile_ids[proto.name] = id;
    return id;
}

void Registry::register_map(const MapProto& proto) {
    m_map = proto;
    m_has_map = true;
}

ItemId Registry::item_id(const std::string& name) const {
    auto it = m_item_ids.find(name);
    return it == m_item_ids.end() ? kNoItem : it->second;
}

ProtoId Registry::recipe_id(const std::string& name) const {
    auto it = m_recipe_ids.find(name);
    return it == m_recipe_ids.end() ? static_cast<ProtoId>(-1) : it->second;
}

ProtoId Registry::technology_id(const std::string& name) const {
    auto it = m_technology_ids.find(name);
    return it == m_technology_ids.end() ? static_cast<ProtoId>(-1) : it->second;
}

ProtoId Registry::entity_id(const std::string& name) const {
    auto it = m_entity_ids.find(name);
    return it == m_entity_ids.end() ? static_cast<ProtoId>(-1) : it->second;
}

TileId Registry::tile_id(const std::string& name) const {
    auto it = m_tile_ids.find(name);
    return it == m_tile_ids.end() ? static_cast<TileId>(-1) : it->second;
}

const MapProto* Registry::map() const { return m_has_map ? &m_map : nullptr; }

const ItemProto* Registry::item(ItemId id) const {
    if (id == kNoItem || id > m_items.size()) return nullptr;
    return &m_items[id - 1];
}

const RecipeProto* Registry::recipe(ProtoId id) const {
    if (id >= m_entities.size() && id >= m_recipes.size()) return nullptr;
    if (id >= m_recipes.size()) return nullptr;
    return &m_recipes[id];
}

const TechnologyProto* Registry::technology(ProtoId id) const {
    if (id >= m_technologies.size()) return nullptr;
    return &m_technologies[id];
}

const EntityProto* Registry::entity(ProtoId id) const {
    if (id >= m_entities.size()) return nullptr;
    return &m_entities[id];
}

const TileProto* Registry::tile(TileId id) const {
    if (id >= m_tiles.size()) return nullptr;
    return &m_tiles[id];
}

ItemId Registry::add_item(const std::string& name) {
    ItemProto proto;
    proto.name = name;
    proto.label = name;
    return register_item(proto);
}

ProtoId Registry::add_recipe(const std::string& name) {
    RecipeProto proto;
    proto.name = name;
    proto.label = name;
    return register_recipe(proto);
}

ProtoId Registry::add_entity(const std::string& name) {
    EntityProto proto;
    proto.name = name;
    proto.label = name;
    return register_entity(proto);
}

TileId Registry::add_tile(const std::string& name) {
    TileProto proto;
    proto.name = name;
    proto.label = name;
    return register_tile(proto);
}

void World::clear() {
    m_crafted = 0;
    m_mined = 0;
    m_produced = 0;
    m_tiles.clear();
    m_ore.clear();
    m_entities.clear();
    m_players.clear();
    m_research_units = 0.0f;
    m_research_level = 0;
    m_active_technology = 0;
    m_researched.clear();
    m_modifiers.clear();
    m_ready = false;
    m_dirty = true;
}

void World::generate(std::uint32_t seed) {
    const Registry& registry = Registry::instance();
    const MapProto* map = registry.map();
    if (!map) {
        m_ready = false;
        return;
    }

    m_seed = seed;
    m_width = std::min(map->width, kMaxWidth);
    m_height = std::min(map->height, kMaxHeight);
    m_tiles.assign(static_cast<std::size_t>(m_width) * m_height, 0);
    m_ore.assign(static_cast<std::size_t>(m_width) * m_height, 0);

    const int cx_center = m_width / 2;
    const int cy_center = m_height / 2;

    Rng rng(seed ^ 0x5DEECE66Du);

    float total_weight = 0.0f;
    for (const MapSurface& surface : map->surface) total_weight += std::max(0.0f, surface.weight);
    if (total_weight <= 0.0f) total_weight = 1.0f;

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            float pick = rng.unit() * total_weight;
            TileId tile = 0;
            for (const MapSurface& surface : map->surface) {
                pick -= std::max(0.0f, surface.weight);
                if (pick <= 0.0f) {
                    tile = registry.tile_id(surface.tile);
                    break;
                }
            }
            m_tiles[static_cast<std::size_t>(y) * m_width + x] = tile;
        }
    }

    auto stamp_circle = [&](int cx, int cy, int radius, TileId tile, float chance,
                            int amount_min, int amount_max) {
        const int r2 = radius * radius;
        for (int y = cy - radius; y <= cy + radius; ++y) {
            for (int x = cx - radius; x <= cx + radius; ++x) {
                if (!in_map(x, y)) continue;
                const int dx = x - cx;
                const int dy = y - cy;
                if (dx * dx + dy * dy > r2) continue;
                if (rng.unit() > chance) continue;
                const std::size_t index = static_cast<std::size_t>(y) * m_width + x;
                m_tiles[index] = tile;
                m_ore[index] = amount_max > 0 ? rng.range(amount_min, amount_max) : 0;
            }
        }
    };

    for (const MapFeature& feature : map->features) {
        const TileId tile = registry.tile_id(feature.tile);
        if (tile == static_cast<TileId>(-1)) continue;
        for (int i = 0; i < feature.count; ++i) {
            const int cx = rng.range(4, std::max(5, m_width - 5));
            const int cy = rng.range(4, std::max(5, m_height - 5));
            stamp_circle(cx, cy, feature.radius, tile, feature.chance, feature.amount_min,
                         feature.amount_max);
        }
    }

    for (const MapOre& ore : map->ores) {
        std::string tile_name = ore.tile;
        if (tile_name.empty()) tile_name = ore.item + "_tile";
        const TileId tile = registry.tile_id(tile_name);
        if (tile == static_cast<TileId>(-1)) continue;
        for (int i = 0; i < ore.count; ++i) {
            int cx = rng.range(6, std::max(7, m_width - 7));
            int cy = rng.range(6, std::max(7, m_height - 7));
            if (ore.near_spawn) {
                cx = cx_center + 6;
                cy = cy_center - 2;
            }
            stamp_circle(cx, cy, ore.radius, tile, 0.88f, ore.amount_min, ore.amount_max);
        }
    }

    for (int y = cy_center - map->spawn_clear_radius; y <= cy_center + map->spawn_clear_radius; ++y) {
        for (int x = cx_center - map->spawn_clear_radius; x <= cx_center + map->spawn_clear_radius; ++x) {
            if (!in_map(x, y)) continue;
            const std::size_t index = static_cast<std::size_t>(y) * m_width + x;
            m_tiles[index] = registry.tile_id(map->surface.empty() ? "" : map->surface.front().tile);
            m_ore[index] = 0;
        }
    }

    m_entities.clear();
    m_players.clear();
    m_research_units = 0.0f;
    m_research_level = 0;
    m_researched.clear();
    m_modifiers.clear();
    m_active_technology = next_technology();
    m_ready = true;
    m_dirty = true;
}

bool World::in_map(int x, int y) const {
    return x >= 0 && y >= 0 && x < m_width && y < m_height;
}

TileId World::tile_at(int x, int y) const {
    if (!in_map(x, y)) return 0;
    return m_tiles[static_cast<std::size_t>(y) * m_width + x];
}

int World::tile_ore(int x, int y) const {
    if (!in_map(x, y)) return 0;
    return m_ore[static_cast<std::size_t>(y) * m_width + x];
}

void World::set_tile(int x, int y, TileId tile) {
    if (!in_map(x, y)) return;
    m_tiles[static_cast<std::size_t>(y) * m_width + x] = tile;
    m_dirty = true;
}

Entity* World::entity_at(int x, int y) {
    auto it = m_entities.find(static_cast<std::uint64_t>(y) * 1024ull + static_cast<std::uint64_t>(x));
    return it == m_entities.end() ? nullptr : &it->second;
}

const Entity* World::entity_at(int x, int y) const {
    auto it = m_entities.find(static_cast<std::uint64_t>(y) * 1024ull + static_cast<std::uint64_t>(x));
    return it == m_entities.end() ? nullptr : &it->second;
}

const PlayerState& World::player(std::uint32_t peer) const {
    static const PlayerState empty;
    auto it = m_players.find(peer);
    return it == m_players.end() ? empty : it->second;
}

void World::init_player(std::uint32_t peer) {
    PlayerState& state = m_players[peer];
    state.inventory.clear();
    const MapProto* map = Registry::instance().map();
    if (!map) return;
    for (const auto& [item, count] : map->starter_inventory) {
        const ItemId id = Registry::instance().item_id(item);
        if (id != kNoItem) state.inventory.add(id, count);
    }
}

bool World::build(std::uint32_t peer, int x, int y, ProtoId proto, int dir, std::string* error) {
    const EntityProto* definition = Registry::instance().entity(proto);
    if (!definition) {
        if (error) *error = "Unknown entity";
        return false;
    }
    if (!definition->placeable) {
        if (error) *error = "Cannot be placed";
        return false;
    }
    if (!in_map(x, y)) {
        if (error) *error = "Out of bounds";
        return false;
    }
    if (entity_at(x, y)) {
        if (error) *error = "Cell is occupied";
        return false;
    }
    const TileProto* tile = Registry::instance().tile(tile_at(x, y));
    if (tile && !tile->buildable) {
        if (error) *error = "Cannot build here";
        return false;
    }

    PlayerState& state = m_players[peer];
    for (const auto& [item, count] : definition->cost) {
        if (!state.inventory.has(item, count)) {
            if (error) *error = "Missing materials for " + definition->label;
            return false;
        }
    }
    for (const auto& [item, count] : definition->cost)
        state.inventory.remove(item, count);

    Entity entity;
    entity.proto = proto;
    entity.x = x;
    entity.y = y;
    entity.dir = dir;
    if (!definition->recipes.empty()) {
        for (const std::string& recipe : definition->recipes) {
            const ProtoId id = Registry::instance().recipe_id(recipe);
            if (id != static_cast<ProtoId>(-1)) {
                entity.recipe = static_cast<int>(id);
                break;
            }
        }
    }
    entity.input.slots.reserve(static_cast<std::size_t>(definition->input_slots));
    entity.output.slots.reserve(static_cast<std::size_t>(definition->output_slots));

    m_entities[entity.key()] = entity;
    m_dirty = true;
    ++m_revision;
    return true;
}

bool World::remove(int x, int y, std::string* error) {
    auto it = m_entities.find(static_cast<std::uint64_t>(y) * 1024ull + static_cast<std::uint64_t>(x));
    if (it == m_entities.end()) {
        if (error) *error = "Nothing here";
        return false;
    }
    m_entities.erase(it);
    m_dirty = true;
    ++m_revision;
    return true;
}

bool World::rotate(int x, int y, std::string* error) {
    Entity* entity = entity_at(x, y);
    if (!entity) {
        if (error) *error = "Nothing here";
        return false;
    }
    const EntityProto* definition = Registry::instance().entity(entity->proto);
    if (!definition || !definition->directional) {
        if (error) *error = "Not rotatable";
        return false;
    }
    entity->dir = (entity->dir % 4) + 1;
    m_dirty = true;
    ++m_revision;
    return true;
}

bool World::mine(int x, int y, std::string* error) {
    if (!in_map(x, y)) {
        if (error) *error = "Out of bounds";
        return false;
    }
    const TileId id = tile_at(x, y);
    const TileProto* tile = Registry::instance().tile(id);
    if (!tile || tile->ore_item.empty() || tile_ore(x, y) <= 0) {
        if (error) *error = "Nothing to mine";
        return false;
    }
    const std::size_t index = static_cast<std::size_t>(y) * m_width + x;
    --m_ore[index];
    const ItemId item = Registry::instance().item_id(tile->ore_item);
    if (item != kNoItem) {
        for (auto& [peer, state] : m_players)
            state.inventory.add(item, 1);
    }
    if (m_ore[index] <= 0) {
        std::string depleted_name = tile->depleted_tile;
        if (depleted_name.empty()) depleted_name = tile->name + ":depleted";
        const TileId depleted = Registry::instance().tile_id(depleted_name);
        if (depleted != static_cast<TileId>(-1)) m_tiles[index] = depleted;
    }
    m_dirty = true;
    ++m_revision;
    return true;
}

bool World::set_recipe(int x, int y, ProtoId recipe, std::string* error) {
    Entity* entity = entity_at(x, y);
    if (!entity) {
        if (error) *error = "Nothing here";
        return false;
    }
    const EntityProto* definition = Registry::instance().entity(entity->proto);
    if (!definition) {
        if (error) *error = "Unknown entity";
        return false;
    }
    bool allowed = definition->recipes.empty();
    for (const std::string& name : definition->recipes)
        if (Registry::instance().recipe_id(name) == recipe) allowed = true;
    if (!allowed) {
        if (error) *error = "Recipe not supported";
        return false;
    }
    entity->recipe = static_cast<int>(recipe);
    entity->progress = 0.0f;
    m_dirty = true;
    ++m_revision;
    return true;
}

bool World::accept_item(Entity& entity, ItemId item) const {
    const EntityProto* definition = Registry::instance().entity(entity.proto);
    if (!definition || item == kNoItem) return false;

    for (Capability capability : definition->capabilities) {
        switch (capability) {
            case Capability::Storage:
                if (entity.input.size() < definition->storage_slots) return true;
                break;
            case Capability::Logistic:
                if (entity.held == kNoItem) return true;
                break;
            case Capability::Research: {
                const ItemProto* proto = Registry::instance().item(item);
                if (proto != nullptr && !proto->research_unit.empty()) return true;
                break;
            }
            case Capability::Crafting: {
                if (definition->recipes.empty()) return true;
                for (const std::string& name : definition->recipes) {
                    const RecipeProto* recipe = Registry::instance().recipe(
                        Registry::instance().recipe_id(name));
                    if (!recipe) continue;
                    for (const auto& [ingredient, count] : recipe->ingredients)
                        if (ingredient == item) return true;
                }
                break;
            }
            default:
                break;
        }
    }
    return false;
}

bool World::transfer_item(int from_x, int from_y, int to_x, int to_y, ItemId item) {
    Entity* target = entity_at(to_x, to_y);
    if (!target) return false;
    if (!accept_item(*target, item)) return false;

    const EntityProto* definition = Registry::instance().entity(target->proto);
    if (!definition) return false;

    for (Capability capability : definition->capabilities) {
        if (capability == Capability::Logistic) {
            if (target->held != kNoItem) return false;
            target->held = item;
            return true;
        }
        if (capability == Capability::Storage || capability == Capability::Crafting) {
            target->input.add(item, 1);
            return true;
        }
    }
    return false;
}

void World::push_output(Entity& entity, int x, int y) {
    const EntityProto* definition = Registry::instance().entity(entity.proto);
    if (!definition) return;

    const int dx = entity.dir == 2 ? 1 : entity.dir == 4 ? -1 : 0;
    const int dy = entity.dir == 3 ? 1 : entity.dir == 1 ? -1 : 0;
    const int tx = x + dx;
    const int ty = y + dy;

    while (!entity.output.slots.empty()) {
        const ItemId item = entity.output.slots.front().first;
        if (!transfer_item(x, y, tx, ty, item)) break;
        entity.output.remove(item, 1);
    }
}

void World::tick_mining(Entity& entity, float dt) {
    const TileId id = tile_at(entity.x, entity.y);
    const TileProto* tile = Registry::instance().tile(id);
    if (!tile || tile->ore_item.empty()) return;
    const int amount = tile_ore(entity.x, entity.y);
    if (amount <= 0) return;

    const EntityProto* definition = Registry::instance().entity(entity.proto);
    const float speed = definition ? definition->speed : 1.0f;
    entity.progress += dt * speed * modifier("mining_speed");
    if (entity.progress < 1.0f) return;
    entity.progress -= 1.0f;

    const std::size_t index = static_cast<std::size_t>(entity.y) * m_width + entity.x;
    --m_ore[index];
    const ItemId item = Registry::instance().item_id(tile->ore_item);
    if (item != kNoItem) {
        entity.output.add(item, 1);
        ++m_mined;
    }
    if (m_ore[index] <= 0) {
        std::string depleted_name = tile->depleted_tile;
        if (depleted_name.empty()) depleted_name = tile->name + ":depleted";
        const TileId depleted = Registry::instance().tile_id(depleted_name);
        if (depleted != static_cast<TileId>(-1)) m_tiles[index] = depleted;
    }
    m_dirty = true;
    ++m_revision;
}

void World::tick_crafting(Entity& entity, float dt) {
    const EntityProto* definition = Registry::instance().entity(entity.proto);
    if (!definition) return;

    ProtoId recipe_id_value = entity.recipe >= 0 ? static_cast<ProtoId>(entity.recipe)
                                                 : static_cast<ProtoId>(-1);
    const RecipeProto* recipe = Registry::instance().recipe(recipe_id_value);

    if (!recipe) {
        for (const std::string& name : definition->recipes) {
            const ProtoId id = Registry::instance().recipe_id(name);
            const RecipeProto* candidate = Registry::instance().recipe(id);
            if (!candidate) continue;
            bool ready = true;
            for (const auto& [ingredient, count] : candidate->ingredients)
                if (!entity.input.has(ingredient, count)) ready = false;
            if (!ready) continue;
            recipe = candidate;
            recipe_id_value = id;
            entity.recipe = static_cast<int>(id);
            break;
        }
    }
    if (!recipe) {
        entity.progress = 0.0f;
        return;
    }

    const float speed = definition->speed * modifier("crafting_speed");
    entity.progress += dt * speed / std::max(0.05f, recipe->time);
    if (entity.progress < 1.0f) return;

    for (const auto& [ingredient, count] : recipe->ingredients)
        if (!entity.input.remove(ingredient, count)) return;
    entity.progress -= 1.0f;
    if (recipe->result != kNoItem) {
        entity.output.add(recipe->result, recipe->result_count);
        m_crafted += static_cast<std::uint64_t>(recipe->result_count);
    }
    m_dirty = true;
    ++m_revision;
}

void World::tick_logistic(Entity& entity, float dt) {
    if (entity.held == kNoItem) return;
    const EntityProto* definition = Registry::instance().entity(entity.proto);
    const float speed = definition ? definition->speed : 1.0f;
    entity.progress += dt * speed * modifier("logistic_speed");

    const int dx = entity.dir == 2 ? 1 : entity.dir == 4 ? -1 : 0;
    const int dy = entity.dir == 3 ? 1 : entity.dir == 1 ? -1 : 0;

    while (entity.progress >= 1.0f) {
        const int tx = entity.x + dx;
        const int ty = entity.y + dy;
        Entity* target = entity_at(tx, ty);
        if (!target) {
            entity.progress = 1.0f;
            break;
        }
        if (target->held == kNoItem && accept_item(*target, entity.held)) {
            target->held = entity.held;
            entity.held = kNoItem;
            entity.progress -= 1.0f;
            continue;
        }
        Entity* source = entity_at(entity.x - dx, entity.y - dy);
        if (source && !source->output.slots.empty()) {
            const ItemId item = source->output.slots.front().first;
            if (accept_item(*target, item)) {
                source->output.remove(item, 1);
                target->held = item;
                entity.progress -= 1.0f;
                continue;
            }
        }
        entity.progress = 1.0f;
        break;
    }
}

void World::tick_inserter(Entity& entity, float dt) {
    const EntityProto* definition = Registry::instance().entity(entity.proto);
    const float speed = definition ? definition->speed : 1.0f;
    entity.swing += dt * speed;
    if (entity.swing < 1.0f) return;
    entity.swing = 0.0f;

    const int dx = entity.dir == 2 ? 1 : entity.dir == 4 ? -1 : 0;
    const int dy = entity.dir == 3 ? 1 : entity.dir == 1 ? -1 : 0;

    if (entity.held != kNoItem) {
        const int tx = entity.x + dx;
        const int ty = entity.y + dy;
        Entity* target = entity_at(tx, ty);
        if (target && accept_item(*target, entity.held)) {
            if (target->held == kNoItem && accept_item(*target, entity.held)) {
                target->held = entity.held;
            } else {
                target->input.add(entity.held, 1);
            }
            entity.held = kNoItem;
            m_dirty = true;
            ++m_revision;
        }
        return;
    }

    Entity* source = entity_at(entity.x - dx, entity.y - dy);
    if (!source) return;
    if (!source->output.slots.empty()) {
        const ItemId item = source->output.slots.front().first;
        source->output.remove(item, 1);
        entity.held = item;
        m_dirty = true;
        ++m_revision;
    }
}

void World::tick_research(Entity& entity, float dt) {
    const EntityProto* definition = Registry::instance().entity(entity.proto);
    if (!definition) return;

    ItemId pack = kNoItem;
    for (const auto& [item, count] : entity.input.slots) {
        if (count <= 0) continue;
        const ItemProto* proto = Registry::instance().item(item);
        if (proto && !proto->research_unit.empty()) {
            pack = item;
            break;
        }
    }
    if (pack == kNoItem) return;

    entity.progress += dt * (definition->speed > 0.0f ? definition->speed : 1.0f);
    if (entity.progress < 1.0f) return;
    entity.progress -= 1.0f;
    entity.input.remove(pack, 1);
    add_research(1.0f);
    m_dirty = true;
    ++m_revision;
}

void World::tick(float dt) {
    if (!m_ready) return;

    std::vector<std::uint64_t> keys;
    keys.reserve(m_entities.size());
    for (const auto& [key, entity] : m_entities) keys.push_back(key);

    for (std::uint64_t key : keys) {
        auto it = m_entities.find(key);
        if (it == m_entities.end()) continue;
        Entity& entity = it->second;
        const EntityProto* definition = Registry::instance().entity(entity.proto);
        if (!definition) continue;

        for (Capability capability : definition->capabilities) {
            switch (capability) {
                case Capability::Mining: tick_mining(entity, dt); break;
                case Capability::Crafting: tick_crafting(entity, dt); break;
                case Capability::Logistic: tick_logistic(entity, dt); break;
                case Capability::Inserter: tick_inserter(entity, dt); break;
                case Capability::Research: tick_research(entity, dt); break;
                default: break;
            }
        }

        if (definition->output_slots > 0) push_output(entity, entity.x, entity.y);
    }
}

void World::client_tick(float dt) {
    if (!m_ready) return;
    for (auto& [key, entity] : m_entities) {
        const EntityProto* definition = Registry::instance().entity(entity.proto);
        if (!definition) continue;
        bool logistic = false;
        for (Capability capability : definition->capabilities)
            if (capability == Capability::Logistic) logistic = true;
        if (!logistic || entity.held == kNoItem) continue;
        entity.progress += dt * definition->speed * modifier("logistic_speed");
        if (entity.progress > 1.0f) entity.progress = 1.0f;
    }
}

float World::modifier(const std::string& name) const {
    auto it = m_modifiers.find(name);
    return it == m_modifiers.end() ? 1.0f : it->second;
}

ProtoId World::next_technology() const {
    const Registry& registry = Registry::instance();
    for (std::size_t i = 0; i < registry.technologies().size(); ++i) {
        const ProtoId id = static_cast<ProtoId>(i);
        if (std::find(m_researched.begin(), m_researched.end(), id) != m_researched.end()) continue;
        const TechnologyProto* proto = registry.technology(id);
        if (!proto) continue;
        bool ready = true;
        for (const std::string& prerequisite : proto->prerequisites)
            if (std::find(m_researched.begin(), m_researched.end(),
                          registry.technology_id(prerequisite)) == m_researched.end())
                ready = false;
        if (ready) return id;
    }
    return static_cast<ProtoId>(-1);
}

void World::apply_effects(const TechnologyProto& proto) {
    for (const auto& [name, value] : proto.effects) {
        m_modifiers[name] = modifier(name) * value;
    }
}

void World::add_research(float units) {
    m_research_units += units;
    ProtoId active = m_active_technology;
    while (active != static_cast<ProtoId>(-1)) {
        const TechnologyProto* proto = Registry::instance().technology(active);
        if (!proto) break;
        if (m_research_units < proto->unit_count) break;
        m_research_units -= proto->unit_count;
        m_researched.push_back(active);
        apply_effects(*proto);
        ++m_research_level;
        active = next_technology();
    }
    m_active_technology = active;
    m_dirty = true;
    ++m_revision;
}

bool World::craft(std::uint32_t peer, ProtoId recipe, std::string* error) {
    const RecipeProto* proto = Registry::instance().recipe(recipe);
    if (!proto) {
        if (error) *error = "Unknown recipe";
        return false;
    }
    PlayerState& state = m_players[peer];
    for (const auto& [ingredient, count] : proto->ingredients) {
        if (!state.inventory.has(ingredient, count)) {
            if (error) *error = "Not enough resources";
            return false;
        }
    }
    for (const auto& [ingredient, count] : proto->ingredients)
        state.inventory.remove(ingredient, count);
    if (proto->result != kNoItem) state.inventory.add(proto->result, proto->result_count);
    m_dirty = true;
    ++m_revision;
    return true;
}

bool World::give_item(std::uint32_t peer, ItemId item, int count) {
    if (item == kNoItem || count <= 0) return false;
    m_players[peer].inventory.add(item, count);
    m_dirty = true;
    ++m_revision;
    return true;
}

bool World::take_item(std::uint32_t peer, ItemId item, int count) {
    PlayerState& state = m_players[peer];
    if (!state.inventory.has(item, count)) return false;
    state.inventory.remove(item, count);
    m_dirty = true;
    ++m_revision;
    return true;
}

std::string World::encode_entities() const {
    const Registry& registry = Registry::instance();
    std::vector<std::string> parts;
    for (const auto& [key, entity] : m_entities) {
        const EntityProto* definition = registry.entity(entity.proto);
        std::string recipe;
        if (entity.recipe >= 0) {
            const RecipeProto* proto = registry.recipe(static_cast<ProtoId>(entity.recipe));
            if (proto) recipe = proto->name;
        }
        std::string held;
        if (entity.held != kNoItem) {
            const ItemProto* proto = registry.item(entity.held);
            if (proto) held = proto->name + ":1";
        }
        parts.push_back((definition ? definition->name : std::string("?")) + ","
                        + std::to_string(entity.x) + "," + std::to_string(entity.y) + ","
                        + std::to_string(entity.dir) + "," + recipe + "," + held + ","
                        + encode_items(entity.input, registry) + ","
                        + encode_items(entity.output, registry));
    }
    return join(parts, ';');
}

std::string World::encode_inventory(std::uint32_t peer) const {
    const Registry& registry = Registry::instance();
    return encode_items(player(peer).inventory, registry);
}

std::string World::encode_research() const {
    const Registry& registry = Registry::instance();
    std::vector<std::string> researched;
    for (ProtoId id : m_researched) {
        const TechnologyProto* proto = registry.technology(id);
        if (proto) researched.push_back(proto->name);
    }
    std::vector<std::string> modifiers;
    for (const auto& [name, value] : m_modifiers)
        modifiers.push_back(name + "=" + std::to_string(value));

    std::string active;
    if (m_active_technology != static_cast<ProtoId>(-1)) {
        const TechnologyProto* proto = registry.technology(m_active_technology);
        if (proto) active = proto->name;
    }

    return std::to_string(m_research_level) + "," + std::to_string(m_research_units) + ","
        + active + "," + join(researched, '|') + "," + join(modifiers, '|');
}

void World::decode_entities(const std::string& data) {
    const Registry& registry = Registry::instance();
    m_entities.clear();

    for (const std::string& record : split(data, ';')) {
        const std::vector<std::string> fields = split_fields(record, ',');
        if (fields.size() < 8) continue;
        const ProtoId proto = registry.entity_id(fields[0]);
        if (proto == static_cast<ProtoId>(-1)) continue;

        Entity entity;
        entity.proto = proto;
        entity.x = std::atoi(fields[1].c_str());
        entity.y = std::atoi(fields[2].c_str());
        entity.dir = std::atoi(fields[3].c_str());
        if (!fields[4].empty()) {
            const ProtoId recipe = registry.recipe_id(fields[4]);
            if (recipe != static_cast<ProtoId>(-1)) entity.recipe = static_cast<int>(recipe);
        }
        if (!fields[5].empty()) {
            const std::size_t colon = fields[5].rfind(':');
            if (colon != std::string::npos)
                entity.held = registry.item_id(fields[5].substr(0, colon));
        }
        decode_items(fields[6], entity.input, registry);
        decode_items(fields[7], entity.output, registry);
        m_entities[entity.key()] = entity;
    }
    m_dirty = true;
    ++m_revision;
}

void World::decode_inventory(std::uint32_t peer, const std::string& data) {
    decode_items(data, m_players[peer].inventory, Registry::instance());
    m_dirty = true;
    ++m_revision;
}

void World::decode_research(const std::string& data) {
    const Registry& registry = Registry::instance();
    const std::vector<std::string> fields = split(data, ',');
    if (fields.size() < 5) return;

    m_research_level = std::atoi(fields[0].c_str());
    m_research_units = std::atof(fields[1].c_str());

    m_researched.clear();
    for (const std::string& name : split(fields[3], '|')) {
        if (name.empty()) continue;
        const ProtoId id = registry.technology_id(name);
        if (id != static_cast<ProtoId>(-1)) m_researched.push_back(id);
    }

    m_modifiers.clear();
    for (const std::string& entry : split(fields[4], '|')) {
        const std::size_t equals = entry.find('=');
        if (equals == std::string::npos) continue;
        m_modifiers[entry.substr(0, equals)] = std::atof(entry.c_str() + equals + 1);
    }

    m_active_technology = fields[2].empty() ? static_cast<ProtoId>(-1)
                                             : registry.technology_id(fields[2]);
    m_dirty = true;
    ++m_revision;
}

}
