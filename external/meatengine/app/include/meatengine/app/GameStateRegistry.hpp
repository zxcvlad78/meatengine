#pragma once

#include <meatengine/GameState.hpp>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace me {

class GameStateRegistry {
public:
    GameStateRegistry() = delete;

    using Factory = std::function<std::unique_ptr<GameState>()>;

    static void register_gs(const std::string& id, Factory factory);
    static void change_to(const std::string& id);
    static bool exists(const std::string& id);
    static std::vector<std::string> list();

private:
    static std::unordered_map<std::string, Factory>& factories();
};


struct GameStateAutoReg {
    GameStateAutoReg(const std::string& id, GameStateRegistry::Factory f) {
        GameStateRegistry::register_gs(id, std::move(f));
    }
};

} // namespace me

#define ME_REGISTER_GAME_STATE_IMPL(ID, TYPE, CNT) \
    namespace { \
        const ::me::GameStateAutoReg _me_autoreg_##CNT{ ID, \
            []() -> std::unique_ptr<::me::GameState> { \
                return std::make_unique<TYPE>(); \
            } \
        }; \
    }

#define ME_REGISTER_GAME_STATE(ID, TYPE) \
    ME_REGISTER_GAME_STATE_IMPL(ID, TYPE, __COUNTER__)