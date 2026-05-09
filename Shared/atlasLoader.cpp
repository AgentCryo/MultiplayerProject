#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "atlas.h"
#include "tile.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_render.h"
#include "SDL3_image/SDL_image.h"

using json = nlohmann::json;
using namespace std;

static tileType parseType(const string& t) {
    if (t == "BACKGROUND_CORNER") return tileType::BACKGROUND_CORNER;
    if (t == "BACKGROUND_EDGE")   return tileType::BACKGROUND_EDGE;
    if (t == "BACKGROUND_CENTER") return tileType::BACKGROUND_CENTER;
    if (t == "BACKGROUND_DECOR") return tileType::BACKGROUND_DECOR;
    return tileType::UNKNOWN;
}

unordered_map<string, atlas> atlasLoader::LoadAll(SDL_Renderer& renderer, const string& folder) {

    SDL_SetLogOutputFunction(
        [](void*, int category, SDL_LogPriority priority, const char* message) {
            if (priority == SDL_LOG_PRIORITY_ERROR || priority == SDL_LOG_PRIORITY_CRITICAL) {
                fprintf(stderr, "%s\n", message);
            } else {
                fprintf(stdout, "%s\n", message);
            }
        },
        nullptr
    );
    
    SDL_Log("[ATLAS] Scanning folder: %s", folder.c_str());
    SDL_Log("[ATLAS] Absolute path: %s", filesystem::absolute(folder).string().c_str());
    SDL_Log("[ATLAS] CWD: %s", filesystem::current_path().string().c_str());

    unordered_map<string, atlas> atlases;

    for (auto& entry : filesystem::recursive_directory_iterator(folder)) {

        // Only process .json atlas files
        if (entry.path().extension() != ".json") continue;

        const string jsonPath = entry.path().string();
        const string baseName = entry.path().stem().string();

        filesystem::path pngPath = entry.path();
        pngPath.replace_extension(".png");
        const string pngPathStr = pngPath.string();

        SDL_Log("[ATLAS] Loading '%s'", baseName.c_str());
        SDL_Log("        JSON: %s", jsonPath.c_str());
        SDL_Log("        PNG : %s", pngPathStr.c_str());

        // Load JSON
        json j;
        {
            ifstream file(jsonPath);
            if (!file) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "[ATLAS ERR] Failed to open JSON: %s", jsonPath.c_str());
                continue;
            }
            file >> j;
        }

        atlas atlas;
        atlas.size = j["size"];

        // Load PNG
        SDL_Surface* surf = IMG_Load(pngPathStr.c_str());
        if (!surf) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "[ATLAS ERR] Failed to load PNG '%s': %s",
                         pngPathStr.c_str(), SDL_GetError());
            continue;
        }

        atlas.surface = surf;

        // Create texture
        atlas.texture = SDL_CreateTextureFromSurface(&renderer, surf);
        if (!atlas.texture) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "[ATLAS ERR] Failed to create texture for '%s': %s",
                         pngPathStr.c_str(), SDL_GetError());
            SDL_DestroySurface(surf);
            continue;
        }

        SDL_Log("[ATLAS] Texture OK: %s", pngPathStr.c_str());

        // Parse tile entries
        for (auto& [key, value] : j.items()) {
            if (key == "size") continue;

            tileInfo info;
            info.x = value["x"];
            info.y = value["y"];
            info.type = parseType(value["type"]);

            atlas.tiles.push_back(info);
        }

        SDL_Log("[ATLAS] Loaded %zu tiles for '%s'", atlas.tiles.size(), baseName.c_str());

        atlases[baseName] = atlas;
    }

    SDL_Log("[ATLAS] Finished. Total atlases loaded: %zu", atlases.size());
    return atlases;
}
