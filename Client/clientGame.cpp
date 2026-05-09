#include <algorithm>
#include <unordered_map>
#include <vector>
#include <SDL3/SDL.h>

#include "../Shared/atlas.h"
#include "../Shared/player.h"
#include "../Helpers/DataTypes/vec2.h"
#include "../Shared/Packets/dataPackets.h"

using namespace std;

void client_sendPacket(uint16_t msgId, const void* data, uint32_t size);

unordered_map<string, atlas> atlases;

//Player Stuff
static unordered_map<uint32_t, player> client_players;

static uint32_t localPlayerId = sizeof(uint32_t);

struct Snapshot {
    float x, y;
    float vx, vy;
    uint64_t timestamp;
};

static unordered_map<uint32_t, vector<Snapshot>> snapshotHistory;
//End Player Stuff

void client_onData(uint16_t msgId, const uint8_t* data, uint32_t size) {
    
}

void client_applyPlayerState(const PlayerStatePacket& ps) {
    if (localPlayerId == sizeof(uint32_t)) localPlayerId = ps.id;

    auto& p = client_players[ps.id];

    if (ps.id == localPlayerId) {
        p.x = ps.x;
        p.y = ps.y;
        p.vx = ps.vx;
        p.vy = ps.vy;
        return;
    }

    Snapshot s{};
    s.x = ps.x;
    s.y = ps.y;
    s.vx = ps.vx;
    s.vy = ps.vy;
    s.timestamp = SDL_GetTicks();

    snapshotHistory[ps.id].push_back(s);

    if (snapshotHistory[ps.id].size() > 20)
        snapshotHistory[ps.id].erase(snapshotHistory[ps.id].begin());
}

void client_removePlayer(uint32_t id) {
    client_players.erase(id);
    snapshotHistory.erase(id);
}

void client_load(SDL_Renderer* renderer) {
    atlases = atlasLoader::LoadAll(*renderer, "Data");
}


void client_update(float dt) {
    const bool* keys = SDL_GetKeyboardState(nullptr);

    float vx = 0.0f;
    float vy = 0.0f;

    if (keys[SDL_SCANCODE_W]) vy -= 1.0f;
    if (keys[SDL_SCANCODE_S]) vy += 1.0f;
    if (keys[SDL_SCANCODE_A]) vx -= 1.0f;
    if (keys[SDL_SCANCODE_D]) vx += 1.0f;

    // Send velocity only when changed
    static float lastVx = 0.0f;
    static float lastVy = 0.0f;

    if (vx != lastVx || vy != lastVy) {
        VelocityPacket m{vx, vy};
        client_sendPacket(PKT_Velocity, &m, sizeof(m));
        lastVx = vx;
        lastVy = vy;
    }

    // CLIENT-SIDE PREDICTION
    if (localPlayerId != sizeof(uint32_t)) {
        auto it = client_players.find(localPlayerId);
        if (it != client_players.end()) {
            it->second.x += vx * 120.0f * dt;
            it->second.y += vy * 120.0f * dt;
        }
    }
}

// Rendering
const uint64_t INTERP_DELAY = 100; // ms

void client_render(SDL_Renderer* renderer) {
    // TEMP RENDER TILE ATLAS
    int drawX = 20;
    int drawY = 20;

    for (auto& [name, a] : atlases) {

        int offset = 0;

        for (const auto& tile : a.tiles) {

            SDL_FRect src {
                   (float)tile.x,
                   (float)tile.y,
                (float)a.size,
                (float)a.size
            };

            SDL_FRect dst {
                (float)(drawX + offset),
                   (float)drawY,
                (float)a.size,
                (float)a.size
            };

            SDL_RenderTexture(renderer, a.texture, &src, &dst);

            offset += a.size + 4;
        }

        drawY += a.size + 10; // move down for next atlas
    }

    // END TEMPORARY TILE RENDERING
    
    // Render local player directly (predicted)
    if (localPlayerId != sizeof(uint32_t)) {
        auto it = client_players.find(localPlayerId);
        if (it != client_players.end()) {
            const player& p = it->second;

            SDL_Vertex verts[3];
            verts[0].position = {p.x, p.y - 20};
            verts[1].position = {p.x - 20, p.y + 20};
            verts[2].position = {p.x + 20, p.y + 20};

            for (int i = 0; i < 3; ++i)
                verts[i].color = SDL_FColor{0, 1, 0, 1}; // green = local

            SDL_RenderGeometry(renderer, nullptr, verts, 3, nullptr, 0);
        }
    }

    // Render other players using interpolation
    uint64_t renderTime = SDL_GetTicks() - INTERP_DELAY;

    for (auto& [id, history] : snapshotHistory) {
        if (id == localPlayerId) continue;
        if (history.size() < 2) continue;

        Snapshot* a = nullptr;
        Snapshot* b = nullptr;

        // Find snapshots around renderTime
        for (int i = history.size() - 1; i >= 0; --i) {
            if (history[i].timestamp <= renderTime) {
                a = &history[i];
                if (i + 1 < history.size())
                    b = &history[i + 1];
                break;
            }
        }

        if (!a || !b) continue;

        float t = float(renderTime - a->timestamp) / float(b->timestamp - a->timestamp);
        t = clamp(t, 0.0f, 1.0f);

        float x = a->x + (b->x - a->x) * t;
        float y = a->y + (b->y - a->y) * t;

        SDL_Vertex verts[3];
        verts[0].position = {x, y - 20};
        verts[1].position = {x - 20, y + 20};
        verts[2].position = {x + 20, y + 20};

        for (int i = 0; i < 3; ++i)
            verts[i].color = SDL_FColor{1, 1, 0, 1}; // yellow = others

        SDL_RenderGeometry(renderer, nullptr, verts, 3, nullptr, 0);
    }
}
