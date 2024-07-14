cmake_minimum_required(VERSION 3.14)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-DVK_NO_PROTOTYPES)
add_definitions(-DGLM_FORCE_RADIANS)
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_definitions(-DGLM_FORCE_LEFT_HANDED)
add_definitions(-DImTextureID=uint64_t)
# add_definitions(-DGLM_FORCE_DEFAULT_ALIGNED_GENTYPES)

include_directories("src/Common")
include_directories("src")
FILE(GLOB_RECURSE SOURCES "src/*" "tests/*")
FILE(GLOB IMGUI_SOURCES "../external/imgui/*.h" "../external/imgui/*.cpp")
FILE(GLOB IMGUI_MISC_SOURCES "../external/imgui/misc/cpp/*.h" "../external/imgui/misc/cpp/*.cpp")
list (APPEND SOURCES ${IMGUI_SOURCES})
list (APPEND SOURCES ${IMGUI_MISC_SOURCES})
add_executable(${PROJECT_NAME} ${SOURCES})

set(VULKAN_SDK "$ENV{VULKAN_SDK}")

include_directories("${VULKAN_SDK}/Include/vma/")
find_package(Vulkan REQUIRED FATAL_ERROR)

# 3rd party libs
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw)

# Internal libs
target_link_libraries(${PROJECT_NAME} Jnrlib)

# Conan libs
target_link_libraries(${PROJECT_NAME} boost::boost glm::glm glog::glog effolkronium_random magic_enum::magic_enum nlohmann_json::nlohmann_json EnTT::EnTT assimp::assimp)

if (BUILD_TESTS)
    target_link_libraries(${PROJECT_NAME} gtest_main)
endif(BUILD_TESTS)

make_filters("${SOURCES}")

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

# Add external dependencies
include_directories("../external/glfw/include")
include_directories("../external/imgui/")
include_directories("../external/imgui/misc/cpp")

# Handle shaders
file(GLOB_RECURSE EDITOR_SHADERS "src/Vulkan/Shaders/*.vert" "src/Vulkan/Shaders/*.frag")

foreach(SHADER_SOURCE ${EDITOR_SHADERS})

    get_filename_component(SHADER_FILE_NAME ${SHADER_SOURCE} NAME)
    set(COMPILED_SHADER "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_FILE_NAME}.spv")

    message(STATUS "Found shader: ${SHADER_FILE_NAME} and it will be compiled in ${COMPILED_SHADER}")

    add_custom_command(OUTPUT ${SHADER_FILE_NAME}.spv
                       COMMAND glslc -g ${SHADER_SOURCE} -o ${COMPILED_SHADER}
                       DEPENDS ${SHADER_SOURCE})
    set(target_name "Shader_${SHADER_FILE_NAME}")
    add_custom_target(${target_name} ALL
                      DEPENDS ${COMPILED_SHADER})
    add_dependencies(${PROJECT_NAME} ${target_name})

    install(FILES ${COMPILED_SHADER} DESTINATION "bin/Shaders")

endforeach()

# Install project
install(TARGETS ${PROJECT_NAME}
        DESTINATION "bin")
