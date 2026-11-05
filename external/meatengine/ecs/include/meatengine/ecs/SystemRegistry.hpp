#pragma once
#include <meatengine/SystemPhase.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <entt/entt.hpp>
#include <string>
#include <functional>

namespace me {

class SystemRegistry {
public:
    SystemRegistry() = delete;

    struct Entry { std::string id; int priority; std::function<void()> func; };
    struct PhaseBucket { std::vector<Entry> entries; bool sorted = true; };

    static void emplace(SystemPhase phase, const std::string& id, int priority, std::function<void()> func);

    static void clear();
    static void clear(SystemPhase phase);

    static void remove(const std::string& id);
    static void remove(const std::string& id, SystemPhase phase);

    static bool has(const std::string& id);
    static bool has(const std::string& id, SystemPhase phase);

    static void run(SystemPhase phase);

    static std::vector<std::string> list();
    static std::vector<std::string> list(SystemPhase phase);

private:
    inline static std::array<PhaseBucket, 6> _buckets;
    static PhaseBucket& bucket(SystemPhase phase);

    static void ensure_sorted(SystemPhase phase);
};

struct SystemAutoReg {
    SystemAutoReg(SystemPhase phase, const std::string& id, int priority, std::function<void()> fn)
    {
        SystemRegistry::emplace(phase, id, priority, std::move(fn));
    }
};

} // namespace me

#define ME_REGISTER_SYSTEM_IMPL(ID, PHASE, PRIORITY, FN, CNT) \
    namespace { \
        const ::me::SystemAutoReg _me_sys_autoreg_##CNT{ PHASE, ID, PRIORITY, \
            (FN) \
        }; \
    }

#define ME_REGISTER_SYSTEM(ID, PHASE, PRIORITY, FN) \
    ME_REGISTER_SYSTEM_IMPL(ID, PHASE, PRIORITY, FN, __COUNTER__)