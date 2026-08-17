cmake_minimum_required(VERSION 3.16)
project(SarabotTestRoom C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

# Find SDL3
find_package(SDL3 REQUIRED CONFIG)

# SDL3_image - try pkg-config first (Linux Mint friendly)
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(SDL3_IMAGE IMPORTED_TARGET SDL3_image)
endif()

add_executable(sarabot_test
    main.c
    game.c
    player.c
    level.c
)

target_link_libraries(sarabot_test SDL3::SDL3)

if(SDL3_IMAGE_FOUND)
    target_link_libraries(sarabot_test PkgConfig::SDL3_IMAGE)
    target_compile_definitions(sarabot_test PRIVATE HAS_SDL3_IMAGE)
else()
    # Fallback: try linking directly
    target_link_libraries(sarabot_test SDL3_image)
endif()

# Copy sprite sheet to build dir
add_custom_command(TARGET sarabot_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/sarabot-alpha.png
    $<TARGET_FILE_DIR:sarabot_test>/sarabot-alpha.png
)
