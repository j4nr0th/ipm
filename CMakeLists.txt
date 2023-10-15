cmake_minimum_required(VERSION 3.26)
project(ipm C)

set(CMAKE_C_STANDARD 11)

add_library(ipm STATIC
        source/ipm_common.c
        include/ipm/ipm_common.h
        source/shared_memory.c
        source/shared_memory.h
        source/ipm_platform.c
        source/ipm_platform.h
        source/ipm_error.c
        include/ipm/ipm_error.h
        source/memory_claim.c
        source/memory_claim.h
        source/ipm_memory.c
        include/ipm/ipm_memory.h
        source/internal.h
)


list(APPEND IPM_TEST_FILES tests/test_common.h tests/test_common.c)



enable_testing()
add_executable(ipm_test_single tests/single_test.c ${IPM_TEST_FILES})
target_include_directories(ipm_test_single PRIVATE include)
target_link_libraries(ipm_test_single PRIVATE ipm)
add_test(NAME test_single COMMAND ipm_test_single)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(ipm PRIVATE -Wall -Wextra -Werror)
    add_executable(ipm_test_fork tests/fork_test.c ${IPM_TEST_FILES})
    target_include_directories(ipm_test_fork PRIVATE include)
    target_link_libraries(ipm_test_fork PRIVATE ipm)
    add_test(NAME test_fork COMMAND ipm_test_fork)
endif ()

