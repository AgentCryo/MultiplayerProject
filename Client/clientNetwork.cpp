#include <SDL3/SDL.h>
#include <iostream>
#include <unordered_map>
#include <enet/enet.h>

#include "../Helpers/DataTypes/vec2.h"
#include "../Shared/player.h"
#include "../Shared/Packets/packets.h"
#include "../Shared/Packets/packetBuilder.h"

void client_applyPlayerState(const PlayerStatePacket& ps);
void client_removePlayer(uint32_t id);
void client_load(SDL_Renderer* renderer);
void client_update(float dt);
void client_render(SDL_Renderer* renderer);
void client_onData(uint16_t msgId, const uint8_t* data, uint32_t size);

// Global peer pointer for sending
static ENetPeer* peer = nullptr;

// Send a arb packet to the server
void client_sendPacket(uint16_t msgId, const void* data, uint32_t size) {
    packetBuilder pb;
    pb.write<uint8_t>(PKT_Data);
    pb.write<uint16_t>(msgId);
    pb.write<uint32_t>(size);
    pb.writeBytes(data, size);

    ENetPacket* pkt = enet_packet_create(pb.data.data(), pb.data.size(), 0);
    enet_peer_send(peer, 0, pkt);
}

// SDL init
static bool initSDL(SDL_Window*& window, SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;

    window = SDL_CreateWindow("Multiplayer Client", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, nullptr);
    return renderer != nullptr;
}

// Main client loop
int runClient(const std::string& ip) {
    std::cout << "[CLIENT] Connecting to: " << ip << "\n";

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!initSDL(window, renderer)) return -1;

    enet_initialize();
    ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!client) {
        std::cout << "[CLIENT] Failed to create ENet client host\n";
        return -1;
    }

    ENetAddress address;
    enet_address_set_host(&address, ip.c_str());
    address.port = 7777;

    peer = enet_host_connect(client, &address, 2, 0);
    if (!peer) {
        std::cout << "[CLIENT] No available peers\n";
        return -1;
    }

    ENetEvent netEvent;
    if (enet_host_service(client, &netEvent, 3000) > 0 &&
        netEvent.type == ENET_EVENT_TYPE_CONNECT) {

        std::cout << "[CLIENT] Connected to server!\n";

        packetBuilder pb;
        pb.write<uint8_t>(PKT_Join);
        ENetPacket* pkt = enet_packet_create(pb.data.data(), pb.data.size(), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, pkt);
        enet_host_flush(client);

        client_load(renderer);
    } else {
        std::cout << "[CLIENT] Connection failed.\n";
        return 0;
    }

    uint64_t lastPacketTime = SDL_GetTicks();
    const uint64_t TIMEOUT_MS = 5000;

    bool running = true;
    SDL_Event event;

    uint64_t lastTick = SDL_GetTicks();

    while (running) {
        uint64_t now = SDL_GetTicks();
        float dt = (now - lastTick) / 1000.0f;
        lastTick = now;

        // Network receive
        while (enet_host_service(client, &netEvent, 0) > 0) {
            if (netEvent.type == ENET_EVENT_TYPE_RECEIVE) {

                lastPacketTime = SDL_GetTicks();

                const uint8_t* data = netEvent.packet->data;
                uint8_t type = data[0];
                const uint8_t* payload = data + 1;

                switch (type) {

                    case PKT_PlayerState: {
                        PlayerStatePacket ps{};
                        memcpy(&ps.id, payload, 4);
                        memcpy(&ps.x,  payload + 4, 4);
                        memcpy(&ps.y,  payload + 8, 4);
                        memcpy(&ps.vx, payload + 12, 4);
                        memcpy(&ps.vy, payload + 16, 4);
                        client_applyPlayerState(ps);
                        break;
                    }

                    case PKT_PlayerLeft: {
                        uint32_t id;
                        memcpy(&id, payload, sizeof(uint32_t));
                        client_removePlayer(id);
                        break;
                    }

                    case PKT_Data: {
                        uint16_t msgId;
                        memcpy(&msgId, payload, sizeof(uint16_t));

                        uint32_t size;
                        memcpy(&size, payload + 2, sizeof(uint32_t));

                        const uint8_t* raw = payload + 6;
                        client_onData(msgId, raw, size);
                        break;
                    }
                }

                enet_packet_destroy(netEvent.packet);
            }
            else if (netEvent.type == ENET_EVENT_TYPE_DISCONNECT) {
                std::cout << "[CLIENT] Disconnected from server\n";
                running = false;
            }
        }

        //Sends Heartbeat ping to server
        static uint64_t lastHeartbeat = 0;
        uint64_t nowMs = SDL_GetTicks();

        if (nowMs - lastHeartbeat > 1000) {
            packetBuilder pb;
            pb.write<uint8_t>(PKT_Heartbeat);

            ENetPacket* pkt = enet_packet_create(
                pb.data.data(), pb.data.size(),
                ENET_PACKET_FLAG_UNSEQUENCED
            );

            enet_peer_send(peer, 0, pkt);
            lastHeartbeat = nowMs;
        }

        // Timeout check
        if (SDL_GetTicks() - lastPacketTime > TIMEOUT_MS) {
            std::cout << "[CLIENT] Server timeout: no packets for 5 seconds\n";
            running = false;
        }

        // SDL events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        // Game update & render
        client_update(dt);

        SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
        SDL_RenderClear(renderer);

        client_render(renderer);

        SDL_RenderPresent(renderer);
    }

    enet_peer_disconnect(peer, 0);
    enet_host_flush(client);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
