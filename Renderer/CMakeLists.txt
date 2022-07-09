cmake_minimum_required(VERSION 3.14)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories("include")
FILE(GLOB SOURCES "src/*" "include/*" "tests/*")

add_executable("Renderer" ${SOURCES})
target_link_libraries("Renderer" "jnrlib")
target_link_libraries("Renderer" ${CONAN_LIBS})

if (BUILD_TESTS)
    target_link_libraries("Renderer" gtest_main)
    if (RUN_TESTS)
        enable_testing()
        add_test(NAME "Renderer" COMMAND "Renderer -rt")
    endif(RUN_TESTS)
endif(BUILD_TESTS)

make_filters("${SOURCES}")

set_property(TARGET "Renderer" PROPERTY CXX_STANDARD 20)
set(CMAKE_INSTALL_PREFIX ../bin)