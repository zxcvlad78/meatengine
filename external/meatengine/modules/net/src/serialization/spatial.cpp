#include <meatengine/net/Replication.hpp>
#include <meatengine/net/Serialization.hpp>
#include <meatengine/spatial/cmp/Transform.hpp>

namespace me::net {

    void serialize(BinaryWriter& w, const me::spatial::cmp::Transform& t) {
        w.write(t.position.x);
        w.write(t.position.y);
        w.write(t.rotation.asRadians());
        w.write(t.scale.x);
        w.write(t.scale.y);
    }

    std::vector<uint8_t> serialize(me::spatial::cmp::Transform& t) {
        BinaryWriter w;
        serialize(w, t);
        return w.buffer();
    }

    bool deserialize(BinaryReader& r, me::spatial::cmp::Transform& out) {
        float x, y, rot, sx, sy;
        if (!r.read(x)) return false;
        if (!r.read(y)) return false;
        if (!r.read(rot)) return false;
        if (!r.read(sx)) return false;
        if (!r.read(sy)) return false;

        out.position = {x, y};
        out.rotation = sf::radians(rot);
        out.scale = {sx, sy};
        return true;
    }

} // namespace me::net

ME_REGISTER_NET_CMP(Transform, me::spatial::cmp::Transform)
