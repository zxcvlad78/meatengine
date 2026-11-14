local entities = Factory.control.entities()
local parts = {}
for _, entity in ipairs(entities) do
    parts[#parts + 1] = string.format("%s@%d,%d d%d", entity.name, entity.x, entity.y, entity.dir)
end
table.sort(parts)

print(string.format("[netcheck] role=%s seed=%d entities=%d",
    F.role or "?", Factory.control.seed() or 0, #entities))
print("[netcheck] " .. table.concat(parts, " | "))

local inv = {}
for _, entry in ipairs(Factory.control.inventory()) do
    inv[#inv + 1] = string.format("%s=%d", entry.item, entry.count)
end
table.sort(inv)
print("[netcheck] inventory: " .. table.concat(inv, ", "))

local research = Factory.control.research()
print(string.format("[netcheck] research level=%d units=%.1f active=%s",
    research.level, research.units, research.active or "-"))

local map = Factory.control.map()
print(string.format("[netcheck] map entries=%d width=%d height=%d",
    #map, Factory.control.width(), Factory.control.height()))

local r = MainLoop.get_registry()
r:foreach_TileMap(function(e, tm)
    local valid = 0
    for _, v in ipairs(tm.tiles) do
        if v >= 0 then valid = valid + 1 end
    end
    print(string.format("[netcheck] tilemap %dx%d tiles=%d valid=%d",
        tm.width, tm.height, #tm.tiles, valid))
end)

r:foreach_TileMap(function(e, tm)
    local tr = r:get_Transform(e)
    print(string.format("[netcheck] tilemap transform=%.1f,%.1f",
        tr.position.x, tr.position.y))
end)
r:foreach_Camera(function(e, c)
    local tr = r:get_Transform(e)
    local cx, cy = 0, 0
    if c.view ~= nil then
        cx = c.view:getCenter().x
        cy = c.view:getCenter().y
    end
    print(string.format("[netcheck] camera pos=%.1f,%.1f zoom=%.2f center=%.1f,%.1f",
        tr.position.x, tr.position.y, c.zoom,
        cx, cy))
end)
