#ifndef MULTIPLAYERPROJECT_PLAYER_H
#define MULTIPLAYERPROJECT_PLAYER_H

#pragma once
#include <cstdint>

struct player {
public:
    uint32_t id{};
    float x  = 0;
    float y  = 0;
    float vx = 0;
    float vy = 0;
};

struct PlayerStatePacket {
    uint32_t id;
    float x;
    float y;
    float vx;
    float vy;
};

struct PlayerLeftPacket {
    uint32_t id;
};

#endif //MULTIPLAYERPROJECT_PLAYER_H