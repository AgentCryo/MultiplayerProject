#ifndef MULTIPLAYERPROJECT_PACKETS_H
#define MULTIPLAYERPROJECT_PACKETS_H

#pragma once
#include <cstdint>

enum PacketType : uint8_t {
    PKT_Join        = 1,
    PKT_Data        = 2,
    PKT_PlayerState = 3,
    PKT_WorldState  = 4,
    PKT_Heartbeat   = 5,
    PKT_PlayerLeft  = 6
};

#endif //MULTIPLAYERPROJECT_PACKETS_H