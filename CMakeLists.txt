cmake_minimum_required(VERSION 3.14)

project("Rendering-Engine")

# Options
option(BUILD_TESTS "Build tests" OFF)

# Handle options
if(BUILD_TESTS)
    message("-- Building tests too")
    add_definitions(-DBUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
else()
    message("-- Not building tests")
endif(BUILD_TESTS)

# Useful functions
macro(make_filters _source_list)
    foreach(_source IN ITEMS ${_source_list})
        get_filename_component(_source_path "${_source}" PATH)
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
        string(REPLACE "/" "\\" _group_path "${_group_path}")
        source_group("${_group_path}" FILES "${_source}")
    endforeach()
endmacro()


if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/JMC)
endif()

# Start building
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
message("-- Build type = ${CMAKE_BUILD_TYPE}")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_INSTALL_PREFIX ../bin)
set(CURRENT_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_definitions(-DAPPLICATION_VERSION="0.0.0")

# Add projects
include_directories("Jnrlib/include")

add_subdirectory("Jnrlib")
add_subdirectory("Renderer")