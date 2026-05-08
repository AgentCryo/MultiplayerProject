#include <enet/enet.h>
#include <thread>
#include <iostream>
#include <unordered_map>
#include "SDL3/SDL_timer.h"

#include "../Shared/player.h"
#include "../Shared/Packets/packets.h"
#include "../Shared/Packets/packetBuilder.h"

void addPlayer(uint32_t id);
void removePlayer(uint32_t id);
void server_update(float dt);
const std::unordered_map<uint32_t, player>& getPlayers();
void server_onData(uint32_t playerId, uint16_t msgId, const uint8_t* data, uint32_t size);

// Timeout tracking
static std::unordered_map<uint32_t, uint64_t> lastHeard;
const uint64_t SERVER_TIMEOUT_MS = 5000;

ENetHost* server = nullptr;

// Send an arb packet to a specific client
void server_sendTo(uint32_t playerId, uint16_t msgId, const void* data, uint32_t size) {
    ENetPeer* peer = &server->peers[playerId];

    packetBuilder pb;
    pb.write<uint8_t>(PKT_Data);
    pb.write<uint16_t>(msgId);
    pb.write<uint32_t>(size);
    pb.writeBytes(data, size);

    ENetPacket* pkt = enet_packet_create(pb.data.data(), pb.data.size(), 0);
    enet_peer_send(peer, 0, pkt);
}

// Broadcast an arb packet to all clients
void server_broadcast(uint16_t msgId, const void* data, uint32_t size) {
    packetBuilder pb;
    pb.write<uint8_t>(PKT_Data);
    pb.write<uint16_t>(msgId);
    pb.write<uint32_t>(size);
    pb.writeBytes(data, size);

    ENetPacket* pkt = enet_packet_create(pb.data.data(), pb.data.size(), 0);
    enet_host_broadcast(server, 0, pkt);
}

// Main server loop
[[noreturn]] static void serverLoop() {
    ENetEvent event;
    uint64_t lastTick = SDL_GetTicks();
    uint64_t lastBroadcast = SDL_GetTicks();

    while (true) {
        // Network events
        while (enet_host_service(server, &event, 1) > 0) {
            switch (event.type) {

                case ENET_EVENT_TYPE_CONNECT: {
                    uint32_t id = event.peer->incomingPeerID;
                    addPlayer(id);
                    lastHeard[id] = SDL_GetTicks();
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT: {
                    uint32_t id = event.peer->incomingPeerID;
                    std::cout << "[SERVER] Player " << id << " disconnected\n";

                    removePlayer(id);
                    lastHeard.erase(id);

                    packetBuilder pb;
                    pb.write<uint8_t>(PKT_PlayerLeft);
                    pb.write<uint32_t>(id);

                    ENetPacket* pkt = enet_packet_create(pb.data.data(), pb.data.size(), 0);
                    enet_host_broadcast(server, 0, pkt);
                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                    const uint8_t* data = event.packet->data;
                    uint8_t type = data[0];
                    const uint8_t* payload = data + 1;

                    uint32_t id = event.peer->incomingPeerID;

                    switch (type) {
                        case PKT_Join: {
                            std::cout << "[SERVER] Player " << id << " joined\n";
                            break;
                        }

                        case PKT_Data: {
                            uint16_t msgId;
                            memcpy(&msgId, payload, sizeof(uint16_t));

                            uint32_t size;
                            memcpy(&size, payload + 2, sizeof(uint32_t));

                            const uint8_t* raw = payload + 6;

                            lastHeard[id] = SDL_GetTicks();

                            server_onData(id, msgId, raw, size);
                            break;
                        }

                        case PKT_Heartbeat: {
                            lastHeard[id] = SDL_GetTicks();
                            break;
                        }

                        default:
                            std::cout << "[SERVER] Unknown packet type: " << (int)type << "\n";
                            break;
                    }

                    enet_packet_destroy(event.packet);
                    break;
                }
            }
        }

        // Game tick
        uint64_t now = SDL_GetTicks();
        float dt = (now - lastTick) / 1000.0f;
        lastTick = now;

        server_update(dt);

        // Broadcast player states & timeout checks
        if (now - lastBroadcast >= 50) {
            lastBroadcast = now;

            // Timeout players
            for (auto it = lastHeard.begin(); it != lastHeard.end(); ) {
                if (now - it->second > SERVER_TIMEOUT_MS) {
                    uint32_t id = it->first;
                    std::cout << "[SERVER] Player " << id << " timed out\n";

                    removePlayer(id);

                    packetBuilder pb;
                    pb.write<uint8_t>(PKT_PlayerLeft);
                    pb.write<uint32_t>(id);

                    ENetPacket* pkt = enet_packet_create(pb.data.data(), pb.data.size(), 0);
                    enet_host_broadcast(server, 0, pkt);

                    ENetPeer* peer = &server->peers[id];
                    enet_peer_disconnect(peer, 0);

                    it = lastHeard.erase(it);
                } else {
                    ++it;
                }
            }

            // Broadcast player states
            const auto& players = getPlayers();
            for (auto& [id, p] : players) {
                packetBuilder pb;
                pb.write<uint8_t>(PKT_PlayerState);

                pb.write(p.id);
                pb.write(p.x);
                pb.write(p.y);
                pb.write(p.vx);
                pb.write(p.vy);

                ENetPacket* pkt = enet_packet_create(pb.data.data(), pb.data.size(), 0);
                enet_host_broadcast(server, 0, pkt);
            }
        }
    }
}

// Start server thread
void startServerThread() {
    std::cout << "[SERVER] Starting server thread...\n";

    if (enet_initialize() != 0) {
        std::cout << "[SERVER] ENet init failed\n";
        return;
    }

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 7777;

    server = enet_host_create(&address, 32, 2, 0, 0);
    if (!server) {
        std::cout << "[SERVER] Failed to create server host\n";
        return;
    }

    std::cout << "[SERVER] Server started on port 7777\n";
    std::thread(serverLoop).detach();
}
