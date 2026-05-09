#ifndef MULTIPLAYERPROJECT_ATLAS_H
#define MULTIPLAYERPROJECT_ATLAS_H
#include <string>
#include <unordered_map>
#include <vector>

#include "tile.h"
#include "SDL3/SDL_render.h"

using namespace std;

struct atlas {
    int size{};
    SDL_Surface* surface{};
    SDL_Texture* texture{};
    vector<tileInfo> tiles{};
};

class atlasLoader {
public:
    static unordered_map<string, atlas> LoadAll(SDL_Renderer& renderer, const string& folder);
};

#endif //MULTIPLAYERPROJECT_ATLAS_H