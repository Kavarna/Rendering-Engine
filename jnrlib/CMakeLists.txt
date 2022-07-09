cmake_minimum_required(VERSION 3.14)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("jnrlib")

include_directories("include")
FILE(GLOB SOURCES "src/*" "include/*")

add_library(jnrlib ${SOURCES})
target_link_libraries("jnrlib" ${CONAN_LIBS})

make_filters("${SOURCES}")

set_property(TARGET "jnrlib" PROPERTY CXX_STANDARD 20)

set(CMAKE_INSTALL_PREFIX ../bin)