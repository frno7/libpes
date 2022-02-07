cmake_minimum_required(VERSION 3.0)

include_directories(../include)
add_executable(run-tests run-tests.c sax-tests.c svg-transcoder-tests.c)
target_link_libraries(run-tests libpes m)

# Run tests silently ('make test' or 'ctest')
add_test(NAME run-tests COMMAND run-tests)

# Run tests with verbose information ('make test-v' or 'cmake --build . --target test_v')
add_custom_target(test-v run-tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run-tests -v
    VERBATIM
)