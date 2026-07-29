cmake_minimum_required(VERSION 3.20)

project(FantasyLibrary LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

add_executable(
    FantasyLibrary

    src/main.cpp
    src/Combat.cpp
    src/Enemy.cpp
    src/WordManager.cpp
    src/SaveManager.cpp
    src/MainMenu.cpp
    src/StoryReader.cpp
    src/GameFont.cpp
    src/MemorySystem.cpp
    src/FinalBoss.cpp
    src/QuillSystem.cpp
    src/EnemySprites.cpp
)

target_link_libraries(
    FantasyLibrary
    PRIVATE
    raylib
)

# Release build: launch as a normal Windows application
# without opening a console window.
if(MSVC)
    set_target_properties(
        FantasyLibrary
        PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    target_link_options(
        FantasyLibrary
        PRIVATE
        /ENTRY:mainCRTStartup
    )
endif()

