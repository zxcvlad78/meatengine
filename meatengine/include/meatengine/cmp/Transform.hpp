#pragma once
#include <SFML/System/Vector2.hpp>

#include <meatengine/meatnet/Replication.hpp>
#include <meatengine/meatnet/Serialization.hpp>

namespace me::cmp {
    struct Transform {
        sf::Vector2f position;
        sf::Angle rotation;
        sf::Vector2f scale = {1.f, 1.f};

        Transform& operator=(const Transform& t) {
            if (this != &t) {
                position = t.position;
                rotation = t.rotation;
                scale = t.scale;
            }
            return *this;
        }

        Transform& operator=(const Transform* t) {
            if (t != nullptr && this != t) {
                position = t->position;
                rotation = t->rotation;
                scale = t->scale;
            }
            return *this;
        }
    };

    inline void serialize(me::net::BinaryWriter& w, const Transform& t) {
        w.WriteFloat(t.position.x);
        w.WriteFloat(t.position.y);
        w.WriteFloat(t.rotation.asRadians());
        w.WriteFloat(t.scale.x);
        w.WriteFloat(t.scale.y);
    }

    inline std::vector<uint8_t> serialize(Transform& t) {
        me::net::BinaryWriter w;
        serialize(w, t);
        return w.GetBuffer();
    }

    inline bool deserialize(me::net::BinaryReader& r, Transform& out) {
        float x, y, rot, sx, sy;
        if (!r.ReadFloat(x)) return false;
        if (!r.ReadFloat(y)) return false;
        if (!r.ReadFloat(rot)) return false;
        if (!r.ReadFloat(sx)) return false;
        if (!r.ReadFloat(sy)) return false;

        out.position = {x, y};
        out.rotation = sf::radians(rot);
        out.scale = {sx, sy};
        return true;
    }

    // inline bool deserialize(std::vector<uint8_t> bytes, Transform& out) {
    //     me::net::BinaryReader r(bytes);
    //     return deserialize(r, out);
    // }
}

ME_REGISTER_NET_CMP(Transform, me::cmp::Transform)