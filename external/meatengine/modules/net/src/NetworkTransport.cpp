#include <meatengine/net/Network.hpp>

#include "NetworkInternal.hpp"

namespace me::net {

Packet& Network::batch_for(PeerId target) {
    auto it = _batches.find(target);
    if (it != _batches.end()) return it->second;
    return _batches.emplace(target, Packet{}).first->second;
}

Packet& Network::broadcast_batch() {
    return _broadcast_packet;
}

void Network::send_record(PeerId target, MsgType type, SendFlag flag, const void* data, std::size_t size) {
    if (!_batching) {
        Packet packet;
        packet.begin(type, flag);
        packet.writer().write(data, size);
        send_packet(target, packet);
        return;
    }

    Packet& batch = batch_for(target);
    if (batch.type() != MsgType::Batch || batch.flag() != flag) {
        flush(target);
        batch.begin(MsgType::Batch, flag);
    } else if (batch.body_size() >= _max_batch_bytes) {
        flush(target);
        batch.begin(MsgType::Batch, flag);
    }
    batch.record(type, flag, data, size);
}

void Network::send(PeerId target, MsgType type, SendFlag flag, const void* data, std::size_t size) {
    if (_batching) {
        send_record(target, type, flag, data, size);
        return;
    }

    Packet packet;
    packet.begin(type, flag);
    packet.writer().write(data, size);
    send_packet(target, packet);
}

void Network::send(PeerId target, MsgType type, SendFlag flag, const std::vector<std::uint8_t>& payload) {
    send(target, type, flag, payload.data(), payload.size());
}

void Network::send(MsgType type, SendFlag flag, const void* data, std::size_t size, PeerId except) {
    if (_batching) {
        if (except != kInvalidPeerId) {
            Packet packet;
            packet.begin(type, flag);
            packet.writer().write(data, size);
            send_packet_except(packet, except);
            return;
        }
        Packet& batch = broadcast_batch();
        if (batch.type() != MsgType::Batch || batch.flag() != flag) {
            flush_broadcast();
            batch.begin(MsgType::Batch, flag);
        } else if (batch.body_size() >= _max_batch_bytes) {
            flush_broadcast();
            batch.begin(MsgType::Batch, flag);
        }
        batch.record(type, flag, data, size);
        return;
    }

    Packet packet;
    packet.begin(type, flag);
    packet.writer().write(data, size);
    send_packet(packet, except);
}

void Network::send(MsgType type, SendFlag flag, const std::vector<std::uint8_t>& payload, PeerId except) {
    send(type, flag, payload.data(), payload.size(), except);
}

void Network::send_packet(PeerId target, Packet& packet) {
    if (packet.size() <= kEnvelopeHeaderSize) return;

    if (_batching && packet.type() != MsgType::Batch) {
        send_record(target, packet.type(), packet.flag(), packet.body_data(), packet.body_size());
        return;
    }

    patch_envelope(packet.header(), packet.type(), packet.flag(), _seq_counter++);
    deliver(target, packet.data(), packet.size(), packet.flag());
}

void Network::send_packet(Packet& packet, PeerId except) {
    if (packet.size() <= kEnvelopeHeaderSize) return;

    patch_envelope(packet.header(), packet.type(), packet.flag(), _seq_counter++);

    if (_server) {
        const ConnectionID ex = (except == kInvalidPeerId)
            ? k_HSteamNetConnection_Invalid
            : conn_of(except);
        _server->Broadcast(packet.data(), static_cast<std::uint32_t>(packet.size()), ex, packet.flag());
        _stats.messages_sent++;
        _stats.bytes_sent += packet.size();
    } else if (_client) {
        _client->Send(packet.data(), static_cast<std::uint32_t>(packet.size()), packet.flag());
        _stats.messages_sent++;
        _stats.bytes_sent += packet.size();
    }
}

void Network::send_packet_except(Packet& packet, PeerId except) {
    if (packet.size() <= kEnvelopeHeaderSize) return;

    patch_envelope(packet.header(), packet.type(), packet.flag(), _seq_counter++);

    if (_server) {
        for (auto& [id, info] : _peers) {
            if (id == except) continue;
            deliver(id, packet.data(), packet.size(), packet.flag());
        }
    } else if (_client) {
        if (except != kServerPeerId)
            deliver(kServerPeerId, packet.data(), packet.size(), packet.flag());
    }
}

void Network::deliver(PeerId target, const void* data, std::size_t size, SendFlag flag) {
    if (_server) {
        const ConnectionID conn = conn_of(target);
        if (conn == k_HSteamNetConnection_Invalid) {
            _stats.dropped_sends++;
            return;
        }
        _server->SendToClient(conn, data, static_cast<std::uint32_t>(size), flag);
        _stats.messages_sent++;
        _stats.bytes_sent += size;
    } else if (_client) {
        if (target != kServerPeerId) {
            _stats.dropped_sends++;
            return;
        }
        _client->Send(data, static_cast<std::uint32_t>(size), flag);
        _stats.messages_sent++;
        _stats.bytes_sent += size;
    }
}

void Network::flush(PeerId target) {
    auto it = _batches.find(target);
    if (it == _batches.end()) return;

    Packet& packet = it->second;
    if (packet.record_count() == 0) return;

    patch_envelope(packet.header(), packet.type(), packet.flag(), _seq_counter++);
    deliver(target, packet.data(), packet.size(), packet.flag());
    _stats.batches_sent++;
    packet.clear();
}

void Network::flush_broadcast() {
    Packet& packet = _broadcast_packet;
    if (packet.record_count() == 0) return;

    patch_envelope(packet.header(), packet.type(), packet.flag(), _seq_counter++);

    if (_server) {
        _server->Broadcast(packet.data(), static_cast<std::uint32_t>(packet.size()), k_HSteamNetConnection_Invalid, packet.flag());
        _stats.messages_sent++;
        _stats.bytes_sent += packet.size();
    } else if (_client) {
        _client->Send(packet.data(), static_cast<std::uint32_t>(packet.size()), packet.flag());
        _stats.messages_sent++;
        _stats.bytes_sent += packet.size();
    }

    _stats.batches_sent++;
    packet.clear();
}

void Network::flush() {
    for (auto& [peer, packet] : _batches) {
        if (packet.record_count() == 0) continue;

        patch_envelope(packet.header(), packet.type(), packet.flag(), _seq_counter++);
        deliver(peer, packet.data(), packet.size(), packet.flag());
        _stats.batches_sent++;
        packet.clear();
    }
    flush_broadcast();
}

}
