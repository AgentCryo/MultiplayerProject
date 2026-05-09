#include <algorithm>
#include <vector>
#include <cstdint>
#include <iostream>

#include "FastNoiseLite.h"
#include "../../Helpers/DataTypes/vec2.h"
#include "../../Shared/tile.h"

using namespace std;

std::vector<uint8_t> GenerateAsteroidNoise(const vec2 size) {
    const int w = (int)size.x;
    const int h = (int)size.y;
    const int cx = w / 2;
    const int cy = h / 2;

    const float maxRadius = min(w, h) * 0.5f;

    std::vector<uint8_t> img(w * h);

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(0.1f);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {

            // Noise
            float n = noise.GetNoise((float)x, (float)y);
            n = (n + 1.0f) * 0.5f;

            // Circular fade
            float dx = x - cx;
            float dy = y - cy;
            float dist = sqrt(dx*dx + dy*dy);

            //float fade = 1.0f - (dist / maxRadius);
            //fade = clamp(fade, 0.0f, 1.0f);
//
            //// Combine
            //float finalValue = n * fade;

            //if (n > 0) std::cout << "Value Above 0: " << n << std::endl;
            img[y * w + x] = (uint8_t)(n < 0.5 ? 0 : 1);
        }
    }
    
    return img;
}

void GetArea(vector<uint8_t>& img, vector<uint8_t>& area, const vec2 size, const vec2 loc) {
    area[0] = img[ loc.x + (loc.y-1) * size.x - 1 ];
    area[1] = img[ loc.x + (loc.y-1) * size.x     ];
    area[2] = img[ loc.x + (loc.y-1) * size.x + 1 ];
    area[3] = img[ loc.x +  loc.y    * size.x - 1 ];
    area[4] = img[ loc.x +  loc.y    * size.x     ];
    area[5] = img[ loc.x +  loc.y    * size.x + 1 ];
    area[6] = img[ loc.x + (loc.y+1) * size.x - 1 ];
    area[7] = img[ loc.x + (loc.y+1) * size.x     ];
    area[8] = img[ loc.x + (loc.y+1) * size.x + 1 ];
}

struct Rule {
    tileType type;
    uint16_t required;
    uint16_t mask;
};

static const vector<Rule> rules = {

    // CENTER
    { tileType::BACKGROUND_CENTER,
      0b111111111,
      0b111111111 },

    // EDGES
    { tileType::BACKGROUND_EDGE,
      0b000111000,
      0b000111000 },

    { tileType::BACKGROUND_EDGE,
      0b010010010,
      0b010010010 },

    // CORNERS
    { tileType::BACKGROUND_CORNER,
      0b000110110,
      0b000110110 },

    // INVERSE CORNERS
    { tileType::BACKGROUND_INVERSE_CORNER,
      0b100100000,
      0b110100000 },

    // INNER EDGES
    { tileType::BACKGROUND_INNER_EDGE,
      0b110010000,
      0b111010000 },

    // INNER CORNERS
    { tileType::BACKGROUND_INNER_CORNER,
      0b000010010,
      0b110110110 },

    // OUTER→INNER TRANSITIONS
    { tileType::BACKGROUND_OUTER_TO_INNER_TRANSITION,
      0b010100000,
      0b110100000 },
};

uint16_t rotateMask(uint16_t m) {
    uint16_t r = 0;

    if (m & (1<<8)) r |= (1<<2);
    if (m & (1<<7)) r |= (1<<5);
    if (m & (1<<6)) r |= (1<<8);

    if (m & (1<<5)) r |= (1<<1);
    if (m & (1<<4)) r |= (1<<4);
    if (m & (1<<3)) r |= (1<<7);

    if (m & (1<<2)) r |= (1<<0);
    if (m & (1<<1)) r |= (1<<3);
    if (m & (1<<0)) r |= (1<<6);

    return r;
}

vector<asteroidTileInfo> GenerateAsteroidShape(const vec2 size) {

    vector<uint8_t> img = GenerateAsteroidNoise(size);
    vector<uint8_t> area(9);
    vector<asteroidTileInfo> out(size.x * size.y, {0, tileType::UNKNOWN});

    for (int y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x) {

            int idx = x + y * size.x;

            if (img[idx] == 0) {out[idx] = {0, tileType::UNKNOWN};continue;}

            GetArea(img, area, size, vec2(x, y));

            uint16_t mask = (area[0] << 8) | (area[1] << 7) | (area[2] << 6) | (area[3] << 5) | (area[4] << 4) | (area[5] << 3) | (area[6] << 2) | (area[7] << 1) | (area[8] << 0);
            uint16_t rotated = mask;

            // Try all 4 rotations
            for (uint8_t rot = 0; rot < 4; rot++) {
                for (const auto& r : rules) {
                    if ((rotated & r.mask) == r.required) {
                        out[idx] = {rot, r.type};
                        goto nextTile;
                    }
                }
                rotated = rotateMask(rotated);
            }
            out[idx] = {0, tileType::UNKNOWN};

            nextTile:;
        }
    }

    return out;
}
