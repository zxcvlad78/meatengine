#pragma once

#include <entt/entt.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include <meatengine/net/Serialization.hpp>

namespace me::net {

template<typename T> void serialize(BinaryWriter&, const T&);
template<typename T> bool deserialize(BinaryReader&, T&);

struct ReplicatedComponent {
    std::string name;
    entt::id_type id = entt::null;

    void (*write)(BinaryWriter&, entt::registry&, entt::entity) = nullptr;
    bool (*read)(BinaryReader&, entt::registry&, entt::entity) = nullptr;
    bool (*skip)(BinaryReader&) = nullptr;
    bool (*has)(entt::registry&, entt::entity) = nullptr;
};

template<typename T>
void write_component(BinaryWriter& w, entt::registry& r, entt::entity e) {
    serialize(w, r.get<T>(e));
}

template<typename T>
bool read_component(BinaryReader& r, entt::registry& reg, entt::entity e) {
    T tmp{};
    if (!deserialize(r, tmp)) return false;
    if (auto* ex = reg.try_get<T>(e)) *ex = std::move(tmp);
    else reg.emplace<T>(e, std::move(tmp));
    return true;
}

template<typename T>
bool skip_component(BinaryReader& r) {
    T tmp{};
    return deserialize(r, tmp);
}

template<typename T>
bool has_component(entt::registry& r, entt::entity e) {
    return r.all_of<T>(e);
}

class Replication {
public:
    Replication() = delete;

    static std::vector<ReplicatedComponent>& components();

    template<typename T>
    static void register_component(const std::string& name) {
        ReplicatedComponent c;
        c.name = name;
        c.id = entt::type_hash<T>::value();

        c.write = &write_component<T>;
        c.read = &read_component<T>;
        c.skip = &skip_component<T>;
        c.has = &has_component<T>;

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
