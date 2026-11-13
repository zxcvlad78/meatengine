#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include <meatengine/net/Network.hpp>
#include <meatengine/net/Serialization.hpp>

namespace me::net::detail {

std::unordered_map<std::uint32_t, std::string>& packed_ids();

Network::PeerEventCallback& on_peer_connected_cb();
Network::PeerEventCallback& on_peer_disconnected_cb();
bool& auto_cleanup_owned_flag();

void safe_callback(const Network::PeerEventCallback& cb, PeerId pid, const char* tag);

void on_client_connected(ConnectionID conn, const char* addr);
void on_client_disconnected(ConnectionID conn, int reason, const char* dbg);
void on_server_message(PeerId peer, ConnectionID conn, const void* data, std::uint32_t size);
void on_connected();
void on_disconnected(int reason, const char* dbg);
void on_server_message_client(const void* data, std::uint32_t size);

void dispatch_message(MsgType type, PeerId sender, BinaryReader& r);
void dispatch_batch(PeerId sender, BinaryReader& r);

}
