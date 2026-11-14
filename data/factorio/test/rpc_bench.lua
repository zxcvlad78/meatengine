local N = 20000
local t0 = Network.now_ms()
local before = Network.stats()
for i = 1, N do
    Factory.control.give("factorio:iron_plate", 1)
end
local after = Network.stats()
local elapsed = (Network.now_ms() - t0) / 1000
local elapsed_s = elapsed > 0 and elapsed or 0.001
print(string.format("[rpcbench] sent=%d received=%d elapsed=%.3f rate=%.0f rpc/s",
    after.rpcs_sent - before.rpcs_sent,
    after.rpcs_received - before.rpcs_received, elapsed_s, N / elapsed_s))
