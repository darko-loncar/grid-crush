cmake_minimum_required(VERSION 3.16)
project(Gridcrush C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

function(enable_project_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endif()
endfunction()

add_executable(gridcrush
    main.c
    tetroid.c
    matrix.c
    draw.c
    sound.c
    ui.c
    preview.c
    input.c
)
target_link_libraries(gridcrush PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf)
if(NOT WIN32)
    target_link_libraries(gridcrush PRIVATE m)
endif()
enable_project_warnings(gridcrush)

add_custom_command(TARGET gridcrush POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:gridcrush>/assets"
)

enable_testing()

add_executable(matrix_test tests/matrix_test.c matrix.c)
enable_project_warnings(matrix_test)
add_test(NAME matrix_test COMMAND matrix_test)
set_tests_properties(matrix_test PROPERTIES
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(tetroid_test tests/tetroid_test.c tetroid.c matrix.c)
enable_project_warnings(tetroid_test)
add_test(NAME tetroid_test COMMAND tetroid_test)
