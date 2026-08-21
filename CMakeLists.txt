cmake_minimum_required(VERSION 3.16)
project(SDL_Man LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
find_package(SDL3 REQUIRED CONFIG)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3_IMAGE REQUIRED IMPORTED_TARGET SDL3_image)
add_executable(sdl_man
    main.c
    game.c
    player.c
    level.c
)
target_compile_options(sdl_man PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
target_link_libraries(sdl_man PRIVATE
    SDL3::SDL3
    PkgConfig::SDL3_IMAGE
)
add_custom_command(
    TARGET sdl_man
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "$<TARGET_FILE_DIR:sdl_man>/assets/levels"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/sarabot-alpha.png"
    "$<TARGET_FILE_DIR:sdl_man>/assets/sarabot-alpha.png"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/assets/levels/testroom.txt"
    "$<TARGET_FILE_DIR:sdl_man>/assets/levels/testroom.txt"
    VERBATIM
)
