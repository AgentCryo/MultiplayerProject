#ifndef MULTIPLAYERPROJECT_DATAPACKETS_H
#define MULTIPLAYERPROJECT_DATAPACKETS_H
#include <cstdint>

enum DataPacketType : uint8_t {
    PKT_Velocity = 1,
};

struct VelocityPacket {
    float vx;
    float vy;
};

#endif //MULTIPLAYERPROJECT_DATAPACKETS_H