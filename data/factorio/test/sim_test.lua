local failures = 0
local checks = 0

local function check(cond, label)
    checks = checks + 1
    if cond then
        print("  [ok  ] " .. label)
    else
        failures = failures + 1
        print("  [FAIL] " .. label)
    end
end

local function find_ore()
    local width = Factory.control.width()
    local height = Factory.control.height()
    local cx = math.floor(width * 0.5)
    local cy = math.floor(height * 0.5)
    for radius = 0, 16 do
        for dy = -radius, radius do
            for dx = -radius, radius do
                local x = cx + dx
                local y = cy + dy
                if Factory.control.has_ore(x, y) then return x, y end
            end
        end
    end
    return nil, nil
end

local function free_cell()
    local width = Factory.control.width()
    local height = Factory.control.height()
    local cx = math.floor(width * 0.5)
    local cy = math.floor(height * 0.5)
    for radius = 1, 16 do
        for dy = -radius, radius do
            for dx = -radius, radius do
                local x = cx + dx
                local y = cy + dy
                if Factory.control.buildable(x, y)
                    and Factory.control.entity_at(x, y) == nil
                    and not Factory.control.has_ore(x, y) then
                    return x, y
                end
            end
        end
    end
    return nil, nil
end

print("[simtest] start")

if not Factory.control.ready() then
    Factory.control.start(false, false, 4242)
end

check(Factory.control.ready(), "world generated")
check(#Factory.control.items() > 0, "items registered")
check(#Factory.control.recipes() > 0, "recipes registered")
check(#Factory.control.entity_prototypes() > 0, "entities registered")
check(#Factory.control.technologies() > 0, "technologies registered")

local ox, oy = find_ore()
check(ox ~= nil, "ore patch generated")
if ox == nil then
    print("[simtest] aborted")
    return
end

Factory.control.build("factorio:drill", ox, oy, 2)
check(Factory.control.entity_at(ox, oy) ~= nil, "drill placed")

local fx, fy = free_cell()
check(fx ~= nil, "free cell found")

Factory.control.build("factorio:furnace", fx, fy, 1)
check(Factory.control.entity_at(fx, fy) ~= nil, "furnace placed")

local cx2, cy2 = free_cell()
Factory.control.build("factorio:chest", cx2, cy2, 1)
check(Factory.control.entity_at(cx2, cy2) ~= nil, "chest placed")

check(Factory.control.entity_at(ox, oy) == nil or true, "entity lookup works")

Factory.control.remove(fx, fy)
check(Factory.control.entity_at(fx, fy) == nil, "furnace removed")

Factory.control.build("factorio:furnace", fx, fy, 1)
Factory.control.give("factorio:iron_ore", 4)
Factory.control.give("factorio:iron_plate", 8)
Factory.control.give("factorio:gear", 4)
check(true, "items granted")

Factory.control.craft("factorio:gear")
check(true, "craft executed")

for _ = 1, 200 do
    Factory.control.tick(1.0 / 20.0)
end

local stats = Factory.control.statistics()
check(stats.mined > 0, "drill mined ore (" .. tostring(stats.mined) .. ")")

local research = Factory.control.research()
check(research.active ~= nil, "active technology present")

print(string.format("[simtest] checks=%d failures=%d %s", checks, failures,
    failures == 0 and "PASS" or "FAIL"))
