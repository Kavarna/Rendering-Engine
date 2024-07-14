cmake_minimum_required(VERSION 3.14)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("Jnrlib")

FILE(GLOB SOURCES "src/*" "include/*")

add_library(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} boost::boost glm::glm glog::glog effolkronium_random magic_enum::magic_enum pngpp::pngpp)

make_filters("${SOURCES}")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

set(CMAKE_INSTALL_PREFIX ../bin)
