cmake_minimum_required(VERSION 3.20)
project(dcvb-utils VERSION 1.0.0)

add_library(${PROJECT_NAME} INTERFACE)
add_library(dcvb::utils ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

option(DCVB_UTILS_BUILD_TESTS "Build tests for dcvb-utils" OFF )
if (DCVB_UTILS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

