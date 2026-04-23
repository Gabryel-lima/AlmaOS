cmake_minimum_required(VERSION 3.20)
project(AlmaOS NONE)

find_program(MAKE_EXECUTABLE NAMES gmake make REQUIRED)
find_program(MTOOLS_MDIR_EXECUTABLE NAMES mdir REQUIRED)

set(ALMAOS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(RELATIVE_PATH ALMAOS_BUILD_DIR "${ALMAOS_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/build")

function(add_almaos_make_target target_name make_target)
    add_custom_target(${target_name}
        COMMAND ${MAKE_EXECUTABLE} -C ${ALMAOS_SOURCE_DIR} BUILD_DIR=${ALMAOS_BUILD_DIR} ${make_target}
        WORKING_DIRECTORY ${ALMAOS_SOURCE_DIR}
        USES_TERMINAL
        VERBATIM)
endfunction()

add_custom_target(almaos ALL
    COMMAND ${MAKE_EXECUTABLE} -C ${ALMAOS_SOURCE_DIR} BUILD_DIR=${ALMAOS_BUILD_DIR}
    WORKING_DIRECTORY ${ALMAOS_SOURCE_DIR}
    USES_TERMINAL
    VERBATIM)

add_almaos_make_target(stage1 stage1)
add_almaos_make_target(stage2 stage2)
add_almaos_make_target(kernel kernel)
add_almaos_make_target(fat fat)
add_almaos_make_target(run run)
add_almaos_make_target(debug debug)
add_almaos_make_target(test_includes test-includes)
add_almaos_make_target(root root)
add_almaos_make_target(tests tests)

enable_testing()

add_test(NAME kernel_tests
    COMMAND ${MAKE_EXECUTABLE} -C ${ALMAOS_SOURCE_DIR} BUILD_DIR=${ALMAOS_BUILD_DIR} kernel-tests
    WORKING_DIRECTORY ${ALMAOS_SOURCE_DIR})

add_test(NAME floppy_image_contents
    COMMAND ${CMAKE_COMMAND}
            -DMAKE_EXECUTABLE=${MAKE_EXECUTABLE}
            -DSOURCE_DIR=${ALMAOS_SOURCE_DIR}
            -DBUILD_DIR=${ALMAOS_BUILD_DIR}
            -DMDIR_EXECUTABLE=${MTOOLS_MDIR_EXECUTABLE}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_floppy.cmake)
