cmake_minimum_required(VERSION 3.14)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories("src/Helpers")
FILE(GLOB_RECURSE SOURCES "src/*" "tests/*")

add_executable("Renderer" ${SOURCES})

find_package(Vulkan REQUIRED FATAL_ERROR)

target_link_libraries("Renderer" Vulkan::Vulkan)
target_link_libraries("Renderer" glfw)  
target_link_libraries("Renderer" Jnrlib)
target_link_libraries("Renderer" ${CONAN_LIBS})

if (BUILD_TESTS)
    target_link_libraries("Renderer" gtest_main)
endif(BUILD_TESTS)

make_filters("${SOURCES}")

set_target_properties("Renderer" PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

set_property(TARGET "Renderer" PROPERTY CXX_STANDARD 20)
set(CMAKE_INSTALL_PREFIX ../bin)