# Lets Visual Studio open this folder directly (File > Open > Folder) as well as
# through ScarygonTrail.sln. Both routes build the same sources.
cmake_minimum_required(VERSION 3.20)
project(ScarygonTrail LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- SDL3 ------------------------------------------------------------------
if(WIN32)
    # Windows: use the SDL3 copy bundled in vendored/ (x64 only, so build as x64).
    set(SDL3_VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL3")
    foreach(sdl_file
            "${SDL3_VENDOR_DIR}/include/SDL3/SDL.h"
            "${SDL3_VENDOR_DIR}/lib/x64/SDL3.lib"
            "${SDL3_VENDOR_DIR}/lib/x64/SDL3.dll")
        if(NOT EXISTS "${sdl_file}")
            message(FATAL_ERROR "Missing bundled SDL3 file: ${sdl_file}\n"
                                "The vendored/SDL3 folder must contain include/ and lib/x64/ (SDL3.lib + SDL3.dll).")
        endif()
    endforeach()

    add_library(SDL3::SDL3 SHARED IMPORTED)
    set_target_properties(SDL3::SDL3 PROPERTIES
        IMPORTED_IMPLIB   "${SDL3_VENDOR_DIR}/lib/x64/SDL3.lib"
        IMPORTED_LOCATION "${SDL3_VENDOR_DIR}/lib/x64/SDL3.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL3_VENDOR_DIR}/include")
else()
    # Linux / macOS: use an installed SDL3.
    find_package(SDL3 REQUIRED CONFIG)
endif()

# ---- Game ------------------------------------------------------------------
add_executable(ScarygonTrail
    src/main.cpp
    src/Game.cpp
    src/Button.cpp
    src/TextRenderer.cpp
    src/Effects.cpp
    src/Party.cpp
    src/Trail.cpp
    src/NarrativeState.cpp
    src/SceneRenderer.cpp)

target_include_directories(ScarygonTrail PRIVATE src)
target_link_libraries(ScarygonTrail PRIVATE SDL3::SDL3)

if(MSVC)
    target_compile_options(ScarygonTrail PRIVATE /W3)
else()
    target_compile_options(ScarygonTrail PRIVATE -Wall -Wextra)
endif()

# ---- Runtime files next to the executable ----------------------------------
# data/ is refreshed on every build, so edits to data/events.txt are picked up.
add_custom_target(copy_game_data
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:ScarygonTrail>/data"
    COMMENT "Copying data/ next to the executable")
add_dependencies(ScarygonTrail copy_game_data)

if(WIN32)
    add_custom_command(TARGET ScarygonTrail POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SDL3_VENDOR_DIR}/lib/x64/SDL3.dll" "$<TARGET_FILE_DIR:ScarygonTrail>"
        COMMENT "Copying SDL3.dll next to the executable")
endif()

# ---- Optional: balance simulator (no SDL needed) ---------------------------
# Not part of the normal build. Build it with:  cmake --build <dir> --target balance_sim
# Run it from the repo root:                    <dir>/balance_sim data/events.txt 20000
add_executable(balance_sim EXCLUDE_FROM_ALL
    tools/balance_sim.cpp
    src/Trail.cpp
    src/Party.cpp
    src/Effects.cpp
    src/NarrativeState.cpp)
target_include_directories(balance_sim PRIVATE src)
