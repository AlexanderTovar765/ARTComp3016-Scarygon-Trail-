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
set(GAME_LOGIC_SOURCES
    src/Effects.cpp
    src/PartyMember.cpp
    src/Party.cpp
    src/ThresholdWatcher.cpp
    src/MemberWatcher.cpp
    src/Trail.cpp
    src/NarrativeState.cpp
    src/Settings.cpp
    src/PlaytestLog.cpp
    src/AudioManager.cpp
    src/LieTracker.cpp
    src/Difficulty.cpp
    src/Animations.cpp
    src/Progress.cpp)

add_executable(ScarygonTrail
    src/main.cpp
    src/Game.cpp
    src/Button.cpp
    src/TextRenderer.cpp
    src/SceneRenderer.cpp
    src/SceneActors.cpp
    ${GAME_LOGIC_SOURCES})

target_include_directories(ScarygonTrail PRIVATE src)
target_link_libraries(ScarygonTrail PRIVATE SDL3::SDL3)

# ---- Sound (optional) -------------------------------------------------------
# irrKlang cannot be redistributed as an archive, so it is not in the repo. Run
# tools/get-irrklang.ps1 to download it into vendored/irrKlang. Without it the game
# still builds and runs, just silently.
set(IRRKLANG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendored/irrKlang")
set(HAVE_IRRKLANG OFF)
if(WIN32 AND EXISTS "${IRRKLANG_DIR}/include/irrKlang.h" AND EXISTS "${IRRKLANG_DIR}/lib/irrKlang.lib")
    set(HAVE_IRRKLANG ON)
    target_compile_definitions(ScarygonTrail PRIVATE SCARYGON_USE_IRRKLANG)
    target_include_directories(ScarygonTrail PRIVATE "${IRRKLANG_DIR}/include")
    target_link_libraries(ScarygonTrail PRIVATE "${IRRKLANG_DIR}/lib/irrKlang.lib")
endif()
message(STATUS "irrKlang sound: ${HAVE_IRRKLANG}")

if(MSVC)
    target_compile_options(ScarygonTrail PRIVATE /W3)
else()
    target_compile_options(ScarygonTrail PRIVATE -Wall -Wextra)
endif()

# ---- Tests and tools (not part of the normal build) -------------------------
# cmake --build . --target logic_tests && ctest   (run from the repo root: tests read data/ and assets/)
add_executable(logic_tests EXCLUDE_FROM_ALL tests/logic_tests.cpp src/TextRenderer.cpp ${GAME_LOGIC_SOURCES})
target_include_directories(logic_tests PRIVATE src)
target_link_libraries(logic_tests PRIVATE SDL3::SDL3)
enable_testing()
add_test(NAME logic_tests COMMAND logic_tests WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(balance_sim EXCLUDE_FROM_ALL tools/balance_sim.cpp
    src/Effects.cpp src/PartyMember.cpp src/Party.cpp src/ThresholdWatcher.cpp
    src/MemberWatcher.cpp src/Trail.cpp src/NarrativeState.cpp
    src/Difficulty.cpp src/Animations.cpp)
target_include_directories(balance_sim PRIVATE src)

# ---- Runtime files next to the executable ----------------------------------
# data/ is refreshed on every build, so edits to data/events.txt are picked up.
add_custom_target(copy_game_data
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:ScarygonTrail>/data"
    COMMENT "Copying data/ next to the executable")
add_custom_target(copy_game_audio
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/audio" "$<TARGET_FILE_DIR:ScarygonTrail>/assets/audio"
    COMMENT "Copying assets/audio next to the executable")
add_dependencies(ScarygonTrail copy_game_data copy_game_audio)

if(WIN32)
    add_custom_command(TARGET ScarygonTrail POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SDL3_VENDOR_DIR}/lib/x64/SDL3.dll" "$<TARGET_FILE_DIR:ScarygonTrail>"
        COMMENT "Copying SDL3.dll next to the executable")
    if(HAVE_IRRKLANG)
        file(GLOB IRRKLANG_DLLS "${IRRKLANG_DIR}/bin/*.dll")
        foreach(dll ${IRRKLANG_DLLS})
            add_custom_command(TARGET ScarygonTrail POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dll}" "$<TARGET_FILE_DIR:ScarygonTrail>")
        endforeach()
    endif()
endif()
