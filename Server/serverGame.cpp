#include <unordered_map>
#include <functional>
#include "../Shared/player.h"
#include "../Shared/Packets/dataPackets.h"
#include "../Shared/Packets/packets.h"

static std::unordered_map<uint32_t, player> players;

// Arb packet handler registry
using ServerHandler = std::function<void(uint32_t playerId, const uint8_t* data, uint32_t size)>;
static std::unordered_map<uint16_t, ServerHandler> handlers;

// Register a handler
void server_register(uint16_t pktId, ServerHandler fn) {
    handlers[pktId] = fn;
}

void addPlayer(uint32_t id) {
    player p{};
    p.id = id;
    p.x = 200.0f;
    p.y = 150.0f;
    p.vx = 0.0f;
    p.vy = 0.0f;
    players[id] = p;
}

void removePlayer(uint32_t id) {
    players.erase(id);
}

const std::unordered_map<uint32_t, player>& getPlayers() {
    return players;
}

static void handleMove(uint32_t playerId, const uint8_t* data, uint32_t size) {
    if (size != sizeof(VelocityPacket)) return;

    const VelocityPacket* m = reinterpret_cast<const VelocityPacket*>(data);

    auto it = players.find(playerId);
    if (it == players.end()) return;

    it->second.vx = m->vx;
    it->second.vy = m->vy;
}

void server_update(float dt) {
    for (auto& [id, p] : players) {
        p.x += p.vx * 120.0f * dt;
        p.y += p.vy * 120.0f * dt;
    }
}

// Main arb packet dispatcher
void server_onData(uint32_t playerId, uint16_t msgId, const uint8_t* data, uint32_t size) {
    auto it = handlers.find(msgId);
    if (it != handlers.end()) {
        it->second(playerId, data, size);
    }
}

// Register all arb packet types
static bool initialized = [](){
    server_register(PKT_Velocity, handleMove);
    return true;
}();
