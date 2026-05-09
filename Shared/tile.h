#ifndef MULTIPLAYERPROJECT_TILETYPE_H
#define MULTIPLAYERPROJECT_TILETYPE_H

enum class tileType {
    BACKGROUND_CORNER,
    BACKGROUND_INVERSE_CORNER,
    BACKGROUND_INNER_CORNER,
    BACKGROUND_OUTER_TO_INNER_TRANSITION,
    BACKGROUND_EDGE,
    BACKGROUND_INNER_EDGE,
    BACKGROUND_CENTER,
    BACKGROUND_DECOR,
    UNKNOWN
};

struct tileInfo {
    int x{};
    int y{};
    tileType type{};
};

struct asteroidTileInfo {
    uint8_t rotation{}; //0 for non, 1 for right, 2 for 180, 3 for left
    tileType type{};
};

#endif //MULTIPLAYERPROJECT_TILETYPE_H