cmake_minimum_required(VERSION 4.0)
project(MultiplayerProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(GIT_EXECUTABLE "C:/Users/Colton/PortableGit/bin/git.exe")

include(FetchContent)

# Helper function for Git dependencies
function(include_dependency libname gitURL gitTag)
    FetchContent_Declare(${libname}
            GIT_REPOSITORY ${gitURL}
            GIT_TAG        ${gitTag}
            GIT_SHALLOW    TRUE
            GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(${libname})
endfunction()

# SDL3
find_package(SDL3 QUIET)
if (NOT SDL3_FOUND)
    include_dependency(SDL3 https://github.com/libsdl-org/SDL.git release-3.4.8)
endif()

# SDL_image
include_dependency(SDL3_image https://github.com/libsdl-org/SDL_image.git release-3.4.4)

# ENet
include_dependency(ENet https://github.com/lsalzman/enet.git master)

# FastNoiseLite
FetchContent_Declare(
        FastNoiseLite
        GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite.git
        GIT_TAG master
)
FetchContent_MakeAvailable(FastNoiseLite)

# JSON
include_dependency(nlohmann_json https://github.com/nlohmann/json.git v3.11.3)

# Executable
add_executable(MultiplayerProject
        main.cpp
        Client/clientNetwork.cpp
        Server/serverNetwork.cpp
        Helpers/DataTypes/vec2.h
        Shared/player.h
        Shared/Packets/packets.h
        Shared/Packets/PacketBuilder.h
        Client/clientGame.cpp
        Server/serverGame.cpp
        Shared/Packets/dataPackets.h
        Shared/tile.h
        Shared/atlas.h
        Shared/atlasLoader.cpp
        Server/Generation/AsteroidShapeGen.cpp
)

# Include directories
target_include_directories(MultiplayerProject PRIVATE
        ${enet_SOURCE_DIR}/include
        ${fastnoiselite_SOURCE_DIR}/Cpp
)

# Link libraries
target_link_libraries(
        MultiplayerProject PRIVATE
        enet
        SDL3::SDL3
        SDL3_image::SDL3_image
        nlohmann_json::nlohmann_json
)

# Copy SDL3 DLL
add_custom_command(
        TARGET MultiplayerProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:MultiplayerProject>
)

# Copy ENet DLL
add_custom_command(
        TARGET MultiplayerProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:enet>
        $<TARGET_FILE_DIR:MultiplayerProject>
)

# Copy SDL_image DLL
add_custom_command(
        TARGET MultiplayerProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3_image::SDL3_image>
        $<TARGET_FILE_DIR:MultiplayerProject>
)

# Copy Data folder
add_custom_command(
        TARGET MultiplayerProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Data
        $<TARGET_FILE_DIR:MultiplayerProject>/Data 
)