# Factorio mod for meatengine

A fully data-driven factory game: the whole game is a Lua mod, the engine only
provides generic mechanisms (prototypes, inventories, capability modules, event
bus, deterministic world generation and multiplayer replication).

```
data/factorio/
  data.lua        <- data stage: every prototype (items, tiles, entities, recipes, technologies, map)
  init.lua        <- control stage: loads the control parts and registers the update system
  res/            <- control stage: presentation and input
    config.lua        layout constants, decor rules
    packed_entities.lua  generic sprite entities (entity/item/overlay/particle/decor)
    game.lua          runtime state and the bridge to the engine API
    worldgen.lua      terrain entity and decoration placement
    visuals.lua       animations, particles, build ghost
    ui.lua            HUD: inventory, crafting, production, minimap, inspect, hotbar
    input.lua         mouse/keyboard, camera, placement intents
    states.lua        the world state (server/client session setup)
  test/           <- scripts for the debug console
  tools/          <- offline 3D pre-renderer that generates the artwork
```

## Data-driven core

The C++ core (`src/factory` in the godlike project) knows nothing about drills,
furnaces or iron ore. It implements mechanisms only:

| Mechanism | What it does |
|-----------|--------------|
| `Prototype registry` | items, tiles, entities, recipes, technologies and the map are registered from Lua (`Factory.data.extend`) |
| `Capability modules` | `mining`, `crafting`, `storage`, `logistic`, `inserter`, `research` – each entity declares which ones it has, the parameters come from the data |
| `Inventory` | stacks, crafting costs, refunds |
| `Research` | technologies with prerequisites, unit counts and effects that multiply the capability speeds |
| `World generation` | surface weights, features (water, rocks) and ore patches, spawn area and starter inventory – all declared in the map prototype |
| `Multiplayer` | the host owns the simulation, clients send intents and receive the world, inventories and research state |
| `Event bus` | `on_error`, `on_notice` and any custom event can be handled from Lua |

Adding a new machine, item, recipe, tile or technology requires **only** editing
`data/factorio/data.lua` – no C++ changes.

### Prototype examples

```lua
{ type = "item", name = "factorio:iron_plate", label = "Iron plate", stack_size = 100 }

{ type = "entity", name = "factorio:drill", graphics = ".../drill.png",
  capabilities = { "mining" }, speed = 0.83, output_slots = 4,
  overlays = { "factorio:overlay_gear" }, anchor_y = -58 }

{ type = "recipe", name = "factorio:gear", result = "factorio:gear",
  time = 1.5, ingredients = { { item = "factorio:iron_plate", count = 2 } } }

{ type = "technology", name = "factorio:faster_belts", unit_count = 5,
  prerequisites = {}, effects = { { name = "logistic_speed", value = 1.35 } } }

{ type = "map", name = "factorio:map", width = 120, height = 120,
  surface = { { tile = "factorio:grass", weight = 0.85 } },
  features = { { tile = "factorio:water", count = 3, radius = 5 } },
  ores = { { item = "factorio:iron_ore", tile = "factorio:iron", count = 5 } },
  starter_inventory = { { item = "factorio:iron_plate", count = 14 } } }
```

### Lua API

```lua
Factory.data.extend({ ... })                  -- register prototypes
Factory.control.start(server, client, seed)  -- start a session
Factory.control.tick(dt)                     -- advance the simulation (host)
Factory.control.build(name, x, y, dir)       -- place (sent to the host on a client)
Factory.control.remove(x, y)
Factory.control.mine(x, y)
Factory.control.rotate(x, y)
Factory.control.craft(recipe)
Factory.control.set_recipe(x, y, recipe)
Factory.control.give(item, count)
Factory.control.entities()                   -- current world state
Factory.control.inventory()                  -- local player inventory
Factory.control.research()                   -- research state
Factory.control.items() / recipes() / technologies() / entity_prototypes() / tiles()
Factory.control.tile_at(x, y) / buildable(x, y) / has_ore(x, y) / entity_at(x, y)
Factory.control.on_event("on_error", function(e) print(e.message) end)
```

## Graphics

All art is produced by an offline 3D pre-renderer that ships with the mod
(`tools/`): meshes are built from boxes, prisms and gears, projected with an
orthographic camera, shaded with a key light plus ambient fill, and given a
baked soft shadow, a dark outline and a fine grain.

```
python tools/gen_assets.py terrain        # terrain tileset (grass, dirt, sand, stone, water, ores)
python tools/gen_machines.py machines     # drill, furnace, belt, inserter, chest, assembler, lab
python tools/gen_machines.py items decor arrows overlays particles cursors character
```

The grid is 32 px per cell (`FConfig.cell`), the tile set uses the same tile
size, so machines line up with the terrain exactly.

## Controls

| Input | Action |
|-------|--------|
| `1` … `7` | select building (drill, furnace, belt, inserter, chest, assembler, lab) |
| LMB on empty cell | place the selected building |
| LMB on a building | rotate it |
| RMB on ore | mine one unit (hold to keep mining) |
| RMB on a building | remove it |
| `R` | cycle the placement direction |
| `E` | inspect the building under the cursor |
| `Esc` | close the inspect panel |
| `-` / `=` | zoom out / zoom in |

## Multiplayer

The host runs the simulation; clients send intents (place/remove/mine/rotate/
craft) over RPC and receive the world state, their inventory and the research
progress. Both peers generate the same map from the host's seed.

## Tests

The debug console (`` ` `` key) runs scripts directly:

```
lua path::data/factorio/test/sim_test.lua
lua path::data/factorio/test/net_check.lua
```

`sim_test.lua` covers world generation, ore, placement, removal, mining,
crafting and research (16 checks). `net_check.lua` prints the role, seed,
entities, inventory and research state – run it on the host and on a client to
compare the synchronised state.
