#include <unordered_map>
#include <SDL3/SDL.h>

#include "../Shared/player.h"
#include "../Helpers/DataTypes/vec2.h"
#include "../Shared/Packets/dataPackets.h"

void client_sendPacket(uint16_t msgId, const void* data, uint32_t size);

static std::unordered_map<uint32_t, player> client_players;

void client_onData(uint16_t msgId, const uint8_t* data, uint32_t size) {
    // Todo: handle custom messages from server
}

void client_applyPlayerState(const PlayerStatePacket& ps) {
    auto& p = client_players[ps.id];
    p.id = ps.id;
    p.x = ps.x;
    p.y = ps.y;
    p.vx = ps.vx;
    p.vy = ps.vy;
}

void client_removePlayer(uint32_t id) {
    client_players.erase(id);
}

void client_update(float dt) {
    const bool* keys = SDL_GetKeyboardState(nullptr);

    float vx = 0.0f;
    float vy = 0.0f;

    if (keys[SDL_SCANCODE_W]) vy -= 1.0f;
    if (keys[SDL_SCANCODE_S]) vy += 1.0f;
    if (keys[SDL_SCANCODE_A]) vx -= 1.0f;
    if (keys[SDL_SCANCODE_D]) vx += 1.0f;

    static float lastVx = 0.0f;
    static float lastVy = 0.0f;

    // Only send when input changes
    if (vx != lastVx || vy != lastVy) {
        VelocityPacket m{vx, vy};
        client_sendPacket(PKT_Velocity, &m, sizeof(m));

        lastVx = vx;
        lastVy = vy;
    }
}

void client_render(SDL_Renderer* renderer) {
    for (auto& [id, p] : client_players) {
        vec2 pos{p.x, p.y};

        SDL_Vertex verts[3];
        verts[0].position = {pos.x, pos.y - 20};
        verts[1].position = {pos.x - 20, pos.y + 20};
        verts[2].position = {pos.x + 20, pos.y + 20};

        for (int i = 0; i < 3; ++i) {
            verts[i].color = SDL_FColor{1, 1, 0, 1};
        }

        SDL_RenderGeometry(renderer, nullptr, verts, 3, nullptr, 0);
    }
}
