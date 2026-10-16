#pragma once
#include <SFML/Graphics/Transform.hpp>

#include <meatengine/meatnet/Serialization.hpp>

namespace me::cmp {
    struct GlobalTransform { sf::Transform transform; };

    inline void serialize(me::net::BinaryWriter& w, GlobalTransform& gt) {
        const float* m = gt.transform.getMatrix();

        w.WriteFloat(m[0]);
        w.WriteFloat(m[4]);
        w.WriteFloat(m[8]);
        w.WriteFloat(m[1]);
        w.WriteFloat(m[5]);
        w.WriteFloat(m[9]);
        w.WriteFloat(m[2]);
        w.WriteFloat(m[6]);
        w.WriteFloat(m[10]);
    }

    inline std::vector<uint8_t> serialize(GlobalTransform& gt) {
        me::net::BinaryWriter w;
        serialize(w, gt);
        return w.GetBuffer();
    }

    inline bool deserialize(me::net::BinaryReader& r, GlobalTransform& out) {
        float a00, a01, a02;
        float a10, a11, a12;
        float a20, a21, a22;

        if (!r.ReadFloat(a00)) return false;
        if (!r.ReadFloat(a01)) return false;
        if (!r.ReadFloat(a02)) return false;
        if (!r.ReadFloat(a10)) return false;
        if (!r.ReadFloat(a11)) return false;
        if (!r.ReadFloat(a12)) return false;
        if (!r.ReadFloat(a20)) return false;
        if (!r.ReadFloat(a21)) return false;
        if (!r.ReadFloat(a22)) return false;

        out.transform = sf::Transform(
            a00, a01, a02,
            a10, a11, a12,
            a20, a21, a22
        );
        return true;
    }

    inline bool deserialize(std::vector<uint8_t> bytes, GlobalTransform& out) {
        me::net::BinaryReader r(bytes);
        return deserialize(r, out);
    }

}