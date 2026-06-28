cmake_minimum_required(VERSION 3.20)
project(nullify VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Strict warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(FetchContent)

# ---------- SFML 2.6 ----------
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        2.6.2
    GIT_SHALLOW    TRUE
)
set(SFML_BUILD_AUDIO ON  CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SFML)

# ---------- Box2D 2.4 ----------
FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG        v2.4.1
    GIT_SHALLOW    TRUE
)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(box2d)

# ---------- nlohmann/json 3.11 ----------
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(nlohmann_json)

# ---------- Sources ----------
set(SOURCES
    src/main.cpp
    src/core/Game.cpp
    src/core/GameState.cpp
    src/core/EventBus.cpp
    src/core/InputMap.cpp
    src/ecs/World.cpp
    src/ecs/Systems/PhysicsSystem.cpp
    src/player/Player.cpp
    src/player/PlayerStateMachine.cpp
    src/player/WeaponSystem.cpp
    src/world/TileMap.cpp
    src/world/LevelLoader.cpp
    src/world/Camera.cpp
    src/rendering/Renderer.cpp
    src/rendering/SpriteBatch.cpp
    src/rendering/ShaderManager.cpp
    src/rendering/ParallaxSystem.cpp
    src/enemies/NavMesh.cpp
    src/enemies/Pathfinding.cpp
    src/enemies/AIStateMachine.cpp
    src/enemies/EnemyManager.cpp
    src/ui/HUD.cpp
)

add_executable(nullify ${SOURCES})

target_include_directories(nullify PRIVATE src)

target_link_libraries(nullify PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    box2d
    nlohmann_json::nlohmann_json
)

# ---------- Copy assets post-build ----------
add_custom_command(TARGET nullify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:nullify>/assets
    COMMENT "Copying assets to build directory"
)
