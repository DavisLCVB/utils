add_executable(test-result test-result.cpp)
add_executable(test-optional test-optional.cpp)
add_executable(test-error test-error.cpp)

target_link_libraries(test-result PRIVATE dcvb::utils)
target_link_libraries(test-optional PRIVATE dcvb::utils)
target_link_libraries(test-error PRIVATE dcvb::utils)

# Clean code flags
foreach(target test-result test-optional test-error)
    target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )

    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
    )
endforeach()

add_test(NAME test-result COMMAND test-result)
add_test(NAME test-optional COMMAND test-optional)
add_test(NAME test-error COMMAND test-error)
