#ifndef MULTIPLAYERPROJECT_TILETYPE_H
#define MULTIPLAYERPROJECT_TILETYPE_H

enum class tileType {
    BACKGROUND_CORNER,
    BACKGROUND_EDGE,
    BACKGROUND_CENTER,
    BACKGROUND_DECOR,
    UNKNOWN
};

struct tileInfo {
    int x{};
    int y{};
    tileType type{};
};

#endif //MULTIPLAYERPROJECT_TILETYPE_H