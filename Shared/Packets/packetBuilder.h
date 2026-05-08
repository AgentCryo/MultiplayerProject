#ifndef MULTIPLAYERPROJECT_PACKETBUILDER_H
#define MULTIPLAYERPROJECT_PACKETBUILDER_H

#pragma once
#include <cstdint>
#include <vector>
#include <cstring>

struct packetBuilder {
    std::vector<uint8_t> data;

    template<typename T>
    void write(const T& value) {
        size_t old = data.size();
        data.resize(old + sizeof(T));
        std::memcpy(data.data() + old, &value, sizeof(T));
    }

    void writeBytes(const void* src, size_t size) {
        size_t old = data.size();
        data.resize(old + size);
        std::memcpy(data.data() + old, src, size);
    }
};

#endif //MULTIPLAYERPROJECT_PACKETBUILDER_H