cmake_minimum_required(VERSION 3.14)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("Jnrlib")

FILE(GLOB SOURCES "src/*" "include/*")

add_library(Jnrlib ${SOURCES})
target_link_libraries("Jnrlib" ${CONAN_LIBS})

make_filters("${SOURCES}")

set_property(TARGET "Jnrlib" PROPERTY CXX_STANDARD 20)

set(CMAKE_INSTALL_PREFIX ../bin)