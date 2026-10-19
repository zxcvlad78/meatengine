#pragma once

#include <entt/entt.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <meatengine/net/Serialization.hpp>

namespace me::net {

template<typename T> void serialize(BinaryWriter&, const T&);
template<typename T> bool deserialize(BinaryReader&, T&);

struct ReplicatedComponent {
    std::string name;
    entt::id_type id = entt::null;

    std::function<void(BinaryWriter&, entt::registry&, entt::entity)> write;
    std::function<bool(BinaryReader&, entt::registry&, entt::entity)> read;
    std::function<bool(BinaryReader&)> skip;
    std::function<bool(entt::registry&, entt::entity)> has;
};

class Replication {
public:
    Replication() = delete;

    static std::vector<ReplicatedComponent>& components();

    template<typename T>
    static void register_component(const std::string& name) {
        ReplicatedComponent c;
        c.name = name;
        c.id = entt::type_hash<T>::value();

        c.write = [](BinaryWriter& w, entt::registry& r, entt::entity e) {
            serialize(w, r.get<T>(e));
        };

        c.read = [](BinaryReader& r, entt::registry& reg, entt::entity e) -> bool {
            T tmp{};
            if (!deserialize(r, tmp)) return false;
            if (auto* ex = reg.try_get<T>(e)) *ex = std::move(tmp);
            else reg.emplace<T>(e, std::move(tmp));
            return true;
        };

        c.skip = [](BinaryReader& r) -> bool {
            T tmp{};
            return deserialize(r, tmp);
        };

        c.has = [](entt::registry& r, entt::entity e) {
            return r.all_of<T>(e);
        };

        components().push_back(std::move(c));
    }

    static std::uint64_t mask_for(entt::registry& r, entt::entity e);
    static void write_components(BinaryWriter& w, entt::registry& r, entt::entity e, std::uint64_t mask);
    static bool read_components(BinaryReader& r, entt::registry& reg, entt::entity e, std::uint64_t mask);
    static bool skip_components(BinaryReader& r, std::uint64_t mask);
};

template<typename T>
struct ReplicationAutoReg {
    explicit ReplicationAutoReg(const std::string& name) {
        Replication::register_component<T>(name);
    }
};

}

#define ME_REGISTER_NET_CMP(TAG, TYPE) \
    inline const ::me::net::ReplicationAutoReg<TYPE> \
        _me_repl_##TAG { #TAG };

#define ME_REGISTER_NET_CMP_AS(TAG, NAME, TYPE) \
    inline const ::me::net::ReplicationAutoReg<TYPE> \
        _me_repl_##TAG { NAME };