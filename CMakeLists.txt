cmake_minimum_required(VERSION 3.16)
project(SDL_Man LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
# SDL3 core – uses the Config mode (works because you have it installed)
find_package(SDL3 REQUIRED CONFIG)
# SDL3_image – manually find the library and header
find_library(SDL3_IMAGE_LIBRARY
    NAMES SDL3_image SDL3-image # common names
    PATHS /usr/local/lib /usr/lib # adjust if you installed elsewhere
)
find_path(SDL3_IMAGE_INCLUDE_DIR
    NAMES SDL3_image/SDL_image.h
    PATHS /usr/local/include /usr/include
)
if(NOT SDL3_IMAGE_LIBRARY OR NOT SDL3_IMAGE_INCLUDE_DIR)
    message(FATAL_ERROR "SDL3_image not found. Please install it.")
endif()
add_executable(sdl_man
    src/main.c
    src/game.c
    src/player.c
    src/level.c
    src/input.c
    src/collision.c
    src/graphics.c
    src/projectile.c
)
target_compile_options(sdl_man PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
target_include_directories(sdl_man PRIVATE
    ${SDL3_IMAGE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(sdl_man PRIVATE
    SDL3::SDL3
    ${SDL3_IMAGE_LIBRARY}
    m
)
# Copy assets (unchanged)
add_custom_command(
    TARGET sdl_man
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "$<TARGET_FILE_DIR:sdl_man>/assets"
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "$<TARGET_FILE_DIR:sdl_man>/assets/levels"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/assets/sarabot-alpha.png"
    "$<TARGET_FILE_DIR:sdl_man>/assets/sarabot-alpha.png"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/assets/levels/testroom.txt"
    "$<TARGET_FILE_DIR:sdl_man>/assets/levels/testroom.txt"
    VERBATIM
)
