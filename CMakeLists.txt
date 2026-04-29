cmake_minimum_required(VERSION 3.16)
project(ZhenzhuEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Compiler Flags ───────────────────────────────
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_definitions(-DENGINE_DEBUG)

# ── Dependencies (Vendor) ────────────────────────
# Assuming libraries are already built in vendor subfolders
set(RAYLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src")
set(RAYLIB_LIB_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src")

set(BOX2D_INCLUDE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/vendor/box2d/include")
set(BOX2D_LIB_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/vendor/box2d/build/bin")

set(JSON_INCLUDE_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/vendor/nlohmann_json/single_include")
set(ENTT_INCLUDE_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/src")
set(SPDLOG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include")

# ── Engine Library ───────────────────────────────
file(GLOB_RECURSE ENGINE_SOURCES 
    "engine/core/*.cpp"
    "engine/utils/*.cpp"
    "engine/async/*.cpp"
    "engine/resources/*.cpp"
    "engine/assets/*.cpp"
)

add_library(zhenzhu-engine STATIC ${ENGINE_SOURCES})

target_include_directories(zhenzhu-engine PUBLIC
    "engine"
    "${JSON_INCLUDE_DIR}"
    "${ENTT_INCLUDE_DIR}"
    "${SPDLOG_INCLUDE_DIR}"
    "${RAYLIB_INCLUDE_DIR}"
    "${BOX2D_INCLUDE_DIR}"
)

# ── Game Executable ──────────────────────────────
file(GLOB GAME_SOURCES "src/*.cpp")

add_executable(MyGame ${GAME_SOURCES})

target_link_libraries(MyGame 
    PRIVATE zhenzhu-engine
    PRIVATE raylib
    PRIVATE box2d
    PRIVATE GL m pthread dl rt X11
)

target_link_directories(MyGame PRIVATE
    "${RAYLIB_LIB_DIR}"
    "${BOX2D_LIB_DIR}"
)

set_target_properties(MyGame PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
