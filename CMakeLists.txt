cmake_minimum_required(VERSION 4.0)
project(MultiplayerProject)

set(CMAKE_CXX_STANDARD 20)
set(GIT_EXECUTABLE "C:/Users/Colton/PortableGit/bin/git.exe")

include(FetchContent)

function(include_dependency libname gitURL gitTag)
    FetchContent_Declare(${libname}
            GIT_REPOSITORY ${gitURL}
            GIT_TAG        ${gitTag}
            GIT_SHALLOW    TRUE
            GIT_PROGRESS   TRUE
    )
    
    FetchContent_MakeAvailable(${libname})
    
endfunction()

find_package(SDL3 QUIET)
if (NOT SDL3_FOUND)
    include_dependency(SDL3 https://github.com/libsdl-org/SDL.git release-3.4.8)
endif ()

include_dependency(SDL3_image https://github.com/libsdl-org/SDL_image.git release-3.4.4)

include_dependency(ENet https://github.com/lsalzman/enet.git master)

add_executable(MultiplayerProject main.cpp
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
        Shared/atlasLoader.cpp)

add_custom_command(TARGET MultiplayerProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:MultiplayerProject>
)

target_include_directories(MultiplayerProject PRIVATE ${enet_SOURCE_DIR}/include)
include_dependency(nlohmann_json https://github.com/nlohmann/json.git v3.11.3)

add_custom_command(TARGET MultiplayerProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:enet>
        $<TARGET_FILE_DIR:MultiplayerProject>
)

target_link_libraries(MultiplayerProject PRIVATE enet)
target_link_libraries(MultiplayerProject PRIVATE SDL3::SDL3)
target_link_libraries(MultiplayerProject PRIVATE SDL3_image::SDL3_image)
target_link_libraries(MultiplayerProject PRIVATE nlohmann_json::nlohmann_json)

add_custom_command(TARGET MultiplayerProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Data
        $<TARGET_FILE_DIR:MultiplayerProject>/Data
)

add_custom_command(TARGET MultiplayerProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3_image::SDL3_image>
        $<TARGET_FILE_DIR:MultiplayerProject>
)