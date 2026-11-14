#include "FactoryRuntime.hpp"

#include <meatengine/app/MainLoop.hpp>
#include <meatengine/net/Network.hpp>
#include <meatengine/net/Rpc.hpp>
#include <meatengine/scripting/ScriptingServer.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace godlike::factory {

namespace {

sol::state& lua_state() { return me::ScriptingServer::lua(); }

std::string table_string(sol::table table, const char* key, const std::string& fallback = {}) {
    sol::object value = table[key];
    if (value.is<std::string>()) return value.as<std::string>();
    return fallback;
}

float table_float(sol::table table, const char* key, float fallback = 0.0f) {
    sol::object value = table[key];
    if (value.is<float>()) return value.as<float>();
    if (value.is<int>()) return static_cast<float>(value.as<int>());
    if (value.is<double>()) return static_cast<float>(value.as<double>());
    return fallback;
}

int table_int(sol::table table, const char* key, int fallback = 0) {
    sol::object value = table[key];
    if (value.is<int>()) return value.as<int>();
    if (value.is<float>()) return static_cast<int>(value.as<float>());
    if (value.is<double>()) return static_cast<int>(value.as<double>());
    return fallback;
}

bool table_bool(sol::table table, const char* key, bool fallback = false) {
    sol::object value = table[key];
    if (value.is<bool>()) return value.as<bool>();
    return fallback;
}

std::vector<std::string> table_string_list(sol::table table, const char* key) {
    std::vector<std::string> result;
    sol::object value = table[key];
    if (!value.is<sol::table>()) return result;
    sol::table list = value.as<sol::table>();
    for (std::size_t i = 1; i <= list.size(); ++i) {
        sol::object entry = list[i];
        if (entry.is<std::string>()) result.push_back(entry.as<std::string>());
    }
    return result;
}

}

World& world() {
    static World instance;
    return instance;
}

Registry& registry() {
    return Registry::instance();
}

Role& role() {
    static Role value = Role::Single;
    return value;
}

std::uint32_t& local_peer() {
    static std::uint32_t value = 0;
    return value;
}

void refresh_local_peer() {
    local_peer() = me::net::Network::peer_id();
}

std::unordered_map<std::uint32_t, PeerState>& peer_states() {
    static std::unordered_map<std::uint32_t, PeerState> states;
    return states;
}

std::unordered_map<std::string, EventHandler>& event_handlers() {
    static std::unordered_map<std::string, EventHandler> handlers;
    return handlers;
}

void dispatch_event(const std::string& name, sol::table payload) {
    auto it = event_handlers().find(name);
    if (it == event_handlers().end()) return;
    if (!it->second.valid()) return;
    auto result = it->second(payload);
    if (!result.valid()) {
        sol::error err = result;
        std::printf("[Factory] event '%s' failed: %s\n", name.c_str(), err.what());
    }
}

namespace {

void extend_prototype(sol::table proto) {
    const std::string type = table_string(proto, "type");
    const std::string name = table_string(proto, "name");
    if (name.empty()) return;

    if (type == "item") {
        ItemProto item;
        item.name = name;
        item.label = table_string(proto, "label", name);
        item.stack_size = table_int(proto, "stack_size", 100);
        item.icon = table_string(proto, "icon");
        item.research_unit = table_string(proto, "research_unit");
        registry().register_item(item);
        return;
    }

    if (type == "recipe") {
        RecipeProto recipe;
        recipe.name = name;
        recipe.label = table_string(proto, "label", name);
        recipe.result = registry().item_id(table_string(proto, "result"));
        recipe.result_count = table_int(proto, "result_count", 1);
        recipe.time = table_float(proto, "time", 1.0f);
        recipe.category = table_string(proto, "category");
        recipe.hidden = table_bool(proto, "hidden", false);

        sol::object ingredients = proto["ingredients"];
        if (ingredients.is<sol::table>()) {
            sol::table list = ingredients.as<sol::table>();
            for (std::size_t i = 1; i <= list.size(); ++i) {
                sol::object entry = list[i];
                if (!entry.is<sol::table>()) continue;
                sol::table pair = entry.as<sol::table>();
                const ItemId item = registry().item_id(table_string(pair, "item"));
                const int count = table_int(pair, "count", 1);
                if (item != kNoItem && count > 0) recipe.ingredients.emplace_back(item, count);
            }
        }
        registry().register_recipe(recipe);
        return;
    }

    if (type == "technology") {
        TechnologyProto technology;
        technology.name = name;
        technology.label = table_string(proto, "label", name);
        technology.prerequisites = table_string_list(proto, "prerequisites");
        technology.unit_count = table_float(proto, "unit_count", 10.0f);
        sol::object effects = proto["effects"];
        if (effects.is<sol::table>()) {
            sol::table list = effects.as<sol::table>();
            for (std::size_t i = 1; i <= list.size(); ++i) {
                sol::object entry = list[i];
                if (!entry.is<sol::table>()) continue;
                sol::table pair = entry.as<sol::table>();
                technology.effects.emplace_back(table_string(pair, "name"),
                                               table_float(pair, "value", 1.0f));
            }
        }
        registry().register_technology(technology);
        return;
    }

    if (type == "entity") {
        EntityProto entity;
        entity.name = name;
        entity.label = table_string(proto, "label", name);
        entity.width = table_int(proto, "width", 1);
        entity.height = table_int(proto, "height", 1);
        entity.input_slots = table_int(proto, "input_slots", 0);
        entity.output_slots = table_int(proto, "output_slots", 0);
        entity.storage_slots = table_int(proto, "storage_slots", 0);
        entity.speed = table_float(proto, "speed", 1.0f);
        entity.graphics = table_string(proto, "graphics");
        entity.directional = table_bool(proto, "directional", false);
        entity.placeable = table_bool(proto, "placeable", true);
        entity.recipes = table_string_list(proto, "recipes");
        entity.filters = table_string_list(proto, "filters");
        entity.overlays = table_string_list(proto, "overlays");
        sol::object cost = proto["cost"];
        if (cost.is<sol::table>()) {
            sol::table list = cost.as<sol::table>();
            for (std::size_t i = 1; i <= list.size(); ++i) {
                sol::object entry = list[i];
                if (!entry.is<sol::table>()) continue;
                sol::table pair = entry.as<sol::table>();
                const ItemId item = registry().item_id(table_string(pair, "item"));
                const int count = table_int(pair, "count", 1);
                if (item != kNoItem && count > 0) entity.cost.emplace_back(item, count);
            }
        }
        entity.anchor_x = table_float(proto, "anchor_x", -32.0f);
        entity.anchor_y = table_float(proto, "anchor_y", -64.0f);
        entity.overlay_y = table_float(proto, "overlay_y", 0.0f);
        entity.flat = table_bool(proto, "flat", false);
        for (const std::string& capability : table_string_list(proto, "capabilities"))
            entity.capabilities.push_back(capability_from_name(capability));
        registry().register_entity(entity);
        return;
    }

    if (type == "tile") {
        TileProto tile;
        tile.name = name;
        tile.label = table_string(proto, "label", name);
        tile.ore_item = table_string(proto, "ore_item");
        tile.depleted_tile = table_string(proto, "depleted_tile");
        tile.ore_amount = table_int(proto, "ore_amount", 0);
        tile.minable = table_bool(proto, "minable", false);
        tile.buildable = table_bool(proto, "buildable", true);
        tile.graphics = table_string(proto, "graphics");
        registry().register_tile(tile);
        return;
    }

    if (type == "map") {
        MapProto map;
        map.name = name;
        map.width = table_int(proto, "width", 120);
        map.height = table_int(proto, "height", 120);
        map.spawn_clear_radius = table_int(proto, "spawn_clear_radius", 4);

        sol::object surface = proto["surface"];
        if (surface.is<sol::table>()) {
            sol::table list = surface.as<sol::table>();
            for (std::size_t i = 1; i <= list.size(); ++i) {
                sol::object entry = list[i];
                if (!entry.is<sol::table>()) continue;
                sol::table item = entry.as<sol::table>();
                map.surface.push_back({table_string(item, "tile"), table_float(item, "weight", 1.0f)});
            }
        }

        sol::object features = proto["features"];
        if (features.is<sol::table>()) {
            sol::table list = features.as<sol::table>();
            for (std::size_t i = 1; i <= list.size(); ++i) {
                sol::object entry = list[i];
                if (!entry.is<sol::table>()) continue;
                sol::table item = entry.as<sol::table>();
                MapFeature feature;
                feature.tile = table_string(item, "tile");
                feature.count = table_int(item, "count", 1);
                feature.radius = table_int(item, "radius", 2);
                feature.chance = table_float(item, "chance", 1.0f);
                feature.amount_min = table_int(item, "amount_min", 0);
                feature.amount_max = table_int(item, "amount_max", 0);
                map.features.push_back(feature);
            }
        }

        sol::object ores = proto["ores"];
        if (ores.is<sol::table>()) {
            sol::table list = ores.as<sol::table>();
            for (std::size_t i = 1; i <= list.size(); ++i) {
                sol::object entry = list[i];
                if (!entry.is<sol::table>()) continue;
                sol::table item = entry.as<sol::table>();
                MapOre ore;
                ore.item = table_string(item, "item");
                ore.tile = table_string(item, "tile");
                ore.count = table_int(item, "count", 1);
                ore.radius = table_int(item, "radius", 2);
                ore.amount_min = table_int(item, "amount_min", 240);
                ore.amount_max = table_int(item, "amount_max", 720);
                ore.near_spawn = table_bool(item, "near_spawn", false);
                map.ores.push_back(ore);
            }
        }

        sol::object starter = proto["starter_inventory"];
        if (starter.is<sol::table>()) {
            sol::table list = starter.as<sol::table>();
            for (std::size_t i = 1; i <= list.size(); ++i) {
                sol::object entry = list[i];
                if (!entry.is<sol::table>()) continue;
                sol::table pair = entry.as<sol::table>();
                const int count = table_int(pair, "count", 1);
                if (count > 0)
                    map.starter_inventory.emplace_back(table_string(pair, "item"), count);
            }
        }
        registry().register_map(map);
        return;
    }
}

sol::table entity_list() {
    sol::state& lua = lua_state();
    sol::table list = lua.create_table();
    int index = 1;
    for (const auto& [key, entity] : world().entities()) {
        const EntityProto* definition = registry().entity(entity.proto);
        sol::table entry = lua.create_table();
        entry["name"] = definition ? definition->name : "";
        entry["x"] = entity.x;
        entry["y"] = entity.y;
        entry["dir"] = entity.dir;
        entry["label"] = definition ? definition->label : "";

        sol::table output = lua.create_table();
        int item_index = 1;
        for (const auto& [item, count] : entity.output.slots) {
            if (count <= 0) continue;
            const ItemProto* proto = registry().item(item);
            sol::table record = lua.create_table();
            record["item"] = proto ? proto->name : "";
            record["count"] = count;
            output[item_index++] = record;
        }
        entry["output"] = output;

        sol::table input = lua.create_table();
        int input_index = 1;
        for (const auto& [item, count] : entity.input.slots) {
            if (count <= 0) continue;
            const ItemProto* proto = registry().item(item);
            sol::table record = lua.create_table();
            record["item"] = proto ? proto->name : "";
            record["count"] = count;
            input[input_index++] = record;
        }
        entry["input"] = input;

        sol::table held = lua.create_table();
        if (entity.held != kNoItem) {
            const ItemProto* proto = registry().item(entity.held);
            held["item"] = proto ? proto->name : "";
            held["count"] = 1;
        }
        entry["held"] = held;

        entry["progress"] = entity.progress;
        entry["swing"] = entity.swing;
        list[index++] = entry;
    }
    return list;
}

sol::table item_list() {
    sol::state& lua = lua_state();
    sol::table list = lua.create_table();
    int index = 1;
    for (const ItemProto& item : registry().items()) {
        sol::table entry = lua.create_table();
        entry["name"] = item.name;
        entry["label"] = item.label;
        entry["stack_size"] = item.stack_size;
        entry["icon"] = item.icon;
        entry["research_unit"] = item.research_unit;
        list[index++] = entry;
    }
    return list;
}

sol::table recipe_list() {
    sol::state& lua = lua_state();
    sol::table list = lua.create_table();
    int index = 1;
    for (const RecipeProto& recipe : registry().recipes()) {
        sol::table entry = lua.create_table();
        entry["name"] = recipe.name;
        entry["label"] = recipe.label;
        entry["result"] = recipe.result != kNoItem && registry().item(recipe.result)
                              ? registry().item(recipe.result)->name : "";
        entry["result_count"] = recipe.result_count;
        entry["time"] = recipe.time;
        entry["category"] = recipe.category;
        sol::table ingredients = lua.create_table();
        int ingredient_index = 1;
        for (const auto& [item, count] : recipe.ingredients) {
            const ItemProto* proto = registry().item(item);
            sol::table pair = lua.create_table();
            pair["item"] = proto ? proto->name : "";
            pair["count"] = count;
            ingredients[ingredient_index++] = pair;
        }
        entry["ingredients"] = ingredients;
        list[index++] = entry;
    }
    return list;
}

sol::table technology_list() {
    sol::state& lua = lua_state();
    sol::table list = lua.create_table();
    int index = 1;
    for (const TechnologyProto& technology : registry().technologies()) {
        sol::table entry = lua.create_table();
        entry["name"] = technology.name;
        entry["label"] = technology.label;
        entry["unit_count"] = technology.unit_count;
        sol::table prerequisites = lua.create_table();
        int prerequisite_index = 1;
        for (const std::string& name : technology.prerequisites)
            prerequisites[prerequisite_index++] = name;
        entry["prerequisites"] = prerequisites;
        sol::table effects = lua.create_table();
        int effect_index = 1;
        for (const auto& [name, value] : technology.effects) {
            sol::table pair = lua.create_table();
            pair["name"] = name;
            pair["value"] = value;
            effects[effect_index++] = pair;
        }
        entry["effects"] = effects;
        list[index++] = entry;
    }
    return list;
}

sol::table entity_prototype_list() {
    sol::state& lua = lua_state();
    sol::table list = lua.create_table();
    int index = 1;
    for (const EntityProto& entity : registry().entities()) {
        sol::table entry = lua.create_table();
        entry["name"] = entity.name;
        entry["label"] = entity.label;
        entry["graphics"] = entity.graphics;
        entry["directional"] = entity.directional;
        entry["width"] = entity.width;
        entry["height"] = entity.height;
        entry["anchor_x"] = entity.anchor_x;
        entry["anchor_y"] = entity.anchor_y;
        entry["overlay_y"] = entity.overlay_y;
        entry["flat"] = entity.flat;
        sol::table overlays = lua.create_table();
        int overlay_index = 1;
        for (const std::string& name : entity.overlays) overlays[overlay_index++] = name;
        entry["overlays"] = overlays;
        sol::table capabilities = lua.create_table();
        int capability_index = 1;
        for (Capability capability : entity.capabilities)
            capabilities[capability_index++] = std::string(capability_name(capability));
        entry["capabilities"] = capabilities;
        sol::table recipes = lua.create_table();
        int recipe_index = 1;
        for (const std::string& name : entity.recipes) recipes[recipe_index++] = name;
        entry["recipes"] = recipes;
        list[index++] = entry;
    }
    return list;
}

sol::table inventory_list() {
    sol::state& lua = lua_state();
    sol::table list = lua.create_table();
    int index = 1;
    for (const auto& [item, count] : world().player(local_peer()).inventory.slots) {
        if (count <= 0) continue;
        const ItemProto* proto = registry().item(item);
        sol::table entry = lua.create_table();
        entry["item"] = proto ? proto->name : "";
        entry["count"] = count;
        list[index++] = entry;
    }
    return list;
}

sol::table research_state() {
    sol::state& lua = lua_state();
    sol::table out = lua.create_table();
    out["level"] = world().research_level();
    out["units"] = world().research_units();

    const Registry& reg = registry();
    std::string active;
    if (world().active_technology() != static_cast<ProtoId>(-1)) {
        const TechnologyProto* proto = reg.technology(world().active_technology());
        if (proto) active = proto->name;
    }
    out["active"] = active;

    sol::table researched = lua.create_table();
    int index = 1;
    for (ProtoId id : world().researched()) {
        const TechnologyProto* proto = reg.technology(id);
        if (proto) researched[index++] = proto->name;
    }
    out["researched"] = researched;
    return out;
}

sol::table map_data() {
    sol::state& lua = lua_state();
    sol::table list = lua.create_table();
    int index = 1;
    for (int y = 0; y < world().height(); ++y) {
        for (int x = 0; x < world().width(); ++x) {
            const TileId id = world().tile_at(x, y);
            const TileProto* proto = registry().tile(id);
            list[index++] = proto ? proto->name : "";
        }
    }
    return list;
}

sol::table tile_list() {
    sol::state& lua = lua_state();
    sol::table list = lua.create_table();
    int index = 1;
    for (const TileProto& tile : registry().tiles()) {
        sol::table entry = lua.create_table();
        entry["name"] = tile.name;
        entry["label"] = tile.label;
        entry["ore_item"] = tile.ore_item;
        entry["buildable"] = tile.buildable;
        entry["graphics"] = tile.graphics;
        list[index++] = entry;
    }
    return list;
}

}

void bind_data_api(sol::state& lua) {
    sol::table factory = lua["Factory"].valid() ? lua["Factory"].get<sol::table>() : lua.create_table();
    lua["Factory"] = factory;
    sol::table data = lua.create_table();
    factory["data"] = data;

    data.set_function("extend", [](sol::table prototypes) {
        for (std::size_t i = 1; i <= prototypes.size(); ++i) {
            sol::object entry = prototypes[i];
            if (!entry.is<sol::table>()) continue;
            extend_prototype(entry.as<sol::table>());
        }
    });

    data.set_function("clear", []() { registry().clear(); });
}

void bind_control_api(sol::state& lua) {
    sol::table factory = lua["Factory"].valid() ? lua["Factory"].get<sol::table>() : lua.create_table();
    lua["Factory"] = factory;
    sol::table control = lua.create_table();
    factory["control"] = control;

    control.set_function("on_event", [](const std::string& name, sol::protected_function fn) {
        event_handlers()[name] = std::move(fn);
    });

    control.set_function("start", [](bool server, bool client, std::uint32_t seed) {
        role() = server ? Role::Server : (client ? Role::Client : Role::Single);
        local_peer() = me::net::Network::peer_id();
        if (role() == Role::Server || role() == Role::Single) {
            world().generate(seed);
            world().init_player(local_peer());
        }
        return role() == Role::Server || role() == Role::Single;
    });

    control.set_function("tick", [](float dt) {
        refresh_local_peer();
        if (role() == Role::Server || role() == Role::Single) world().tick(dt);
        update_network(dt);
    });

    control.set_function("client_tick", [](float dt) {
        if (role() == Role::Client) world().client_tick(dt);
    });

    control.set_function("request_sync", []() {
        me::net::Rpc::invoke("factory:sync_request");
    });

    control.set_function("on_peer_connected", [](std::uint32_t peer) {
        if (role() != Role::Server) return;
        world().init_player(peer);
        peer_states()[peer] = PeerState{};
        me::net::Rpc::invoke_to(peer, "factory:world", static_cast<std::uint32_t>(world().seed()));
        me::net::Rpc::invoke_to(peer, "factory:entities", world().encode_entities());
        me::net::Rpc::invoke_to(peer, "factory:inventory", world().encode_inventory(peer));
        me::net::Rpc::invoke_to(peer, "factory:research", world().encode_research());
    });

    control.set_function("build", [](const std::string& name, int x, int y, int dir) {
        if (role() == Role::Client) {
            me::net::Rpc::invoke("factory:build", name, x, y, dir);
            return;
        }
        std::string error;
        if (!world().build(local_peer(), x, y, registry().entity_id(name), dir, &error)) {
            sol::state& lua = lua_state();
            sol::table payload = lua.create_table();
            payload["message"] = error;
            dispatch_event("on_error", payload);
        }
    });

    control.set_function("remove", [](int x, int y) {
        if (role() == Role::Client) {
            me::net::Rpc::invoke("factory:remove", x, y);
            return;
        }
        std::string error;
        if (!world().remove(x, y, &error)) {
            sol::state& lua = lua_state();
            sol::table payload = lua.create_table();
            payload["message"] = error;
            dispatch_event("on_error", payload);
        }
    });

    control.set_function("mine", [](int x, int y) {
        if (role() == Role::Client) {
            me::net::Rpc::invoke("factory:mine", x, y);
            return;
        }
        std::string error;
        if (!world().mine(x, y, &error)) {
            sol::state& lua = lua_state();
            sol::table payload = lua.create_table();
            payload["message"] = error;
            dispatch_event("on_error", payload);
        }
    });

    control.set_function("rotate", [](int x, int y) {
        if (role() == Role::Client) {
            me::net::Rpc::invoke("factory:rotate", x, y);
            return;
        }
        std::string error;
        if (!world().rotate(x, y, &error)) {
            sol::state& lua = lua_state();
            sol::table payload = lua.create_table();
            payload["message"] = error;
            dispatch_event("on_error", payload);
        }
    });

    control.set_function("craft", [](const std::string& recipe) {
        if (role() == Role::Client) {
            me::net::Rpc::invoke("factory:craft", recipe);
            return;
        }
        std::string error;
        if (!world().craft(local_peer(), registry().recipe_id(recipe), &error)) {
            sol::state& lua = lua_state();
            sol::table payload = lua.create_table();
            payload["message"] = error;
            dispatch_event("on_error", payload);
        }
    });

    control.set_function("set_recipe", [](int x, int y, const std::string& recipe) {
        if (role() == Role::Client) {
            me::net::Rpc::invoke("factory:set_recipe", x, y, recipe);
            return;
        }
        std::string error;
        if (!world().set_recipe(x, y, registry().recipe_id(recipe), &error)) {
            sol::state& lua = lua_state();
            sol::table payload = lua.create_table();
            payload["message"] = error;
            dispatch_event("on_error", payload);
        }
    });

    control.set_function("give", [](const std::string& item, int count) {
        if (role() == Role::Client) {
            me::net::Rpc::invoke("factory:give", item, count);
            return;
        }
        world().give_item(local_peer(), registry().item_id(item), count);
    });

    control.set_function("entities", &entity_list);
    control.set_function("entity_prototypes", &entity_prototype_list);
    control.set_function("statistics", []() {
        sol::state& lua = lua_state();
        sol::table out = lua.create_table();
        out["crafted"] = world().crafted();
        out["mined"] = world().mined();
        out["buildings"] = static_cast<std::uint64_t>(world().entities().size());
        return out;
    });

    control.set_function("items", &item_list);
    control.set_function("recipes", &recipe_list);
    control.set_function("technologies", &technology_list);
    control.set_function("tiles", &tile_list);
    control.set_function("inventory", &inventory_list);
    control.set_function("peer", []() { return local_peer(); });
    control.set_function("research", &research_state);
    control.set_function("map", &map_data);

    control.set_function("tile_at", [](int x, int y) {
        const TileProto* proto = registry().tile(world().tile_at(x, y));
        return proto ? proto->name : "";
    });

    control.set_function("tile_ore", [](int x, int y) { return world().tile_ore(x, y); });
    control.set_function("buildable", [](int x, int y) {
        const TileProto* proto = registry().tile(world().tile_at(x, y));
        return proto ? proto->buildable : false;
    });
    control.set_function("has_ore", [](int x, int y) {
        const TileProto* proto = registry().tile(world().tile_at(x, y));
        return proto && !proto->ore_item.empty() && world().tile_ore(x, y) > 0;
    });
    control.set_function("entity_at", [](int x, int y) -> sol::object {
        const Entity* entity = world().entity_at(x, y);
        if (!entity) return sol::object(sol::nil);
        const EntityProto* definition = registry().entity(entity->proto);
        sol::table out = lua_state().create_table();
        out["name"] = definition ? definition->name : "";
        return sol::object(out);
    });
    control.set_function("width", []() { return world().width(); });
    control.set_function("height", []() { return world().height(); });
    control.set_function("ready", []() { return world().ready(); });
    control.set_function("seed", []() { return world().seed(); });
}

}
