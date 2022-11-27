cmake_minimum_required(VERSION 3.14)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-DVK_NO_PROTOTYPES)

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

# Handle shaders
file(GLOB_RECURSE EDITOR_SHADERS "src/Editor/Shaders/*.vert" "src/Editor/Shaders/*.frag")

foreach(SHADER_SOURCE ${EDITOR_SHADERS})

    get_filename_component(SHADER_FILE_NAME ${SHADER_SOURCE} NAME)
    message(STATUS "Found shader : ${SHADER_FILE_NAME}")
    set(COMPILED_SHADER "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_FILE_NAME}.spv")

    add_custom_command(OUTPUT ${SHADER_FILE_NAME}.spv
                       COMMAND glslc ${SHADER_SOURCE} -o ${COMPILED_SHADER}
                       DEPENDS ${SHADER_SOURCE})
    set(target_name "Shader_${SHADER_FILE_NAME}")
    add_custom_target(${target_name} ALL
                      DEPENDS ${COMPILED_SHADER})
    add_dependencies("Renderer" ${target_name})

    install(FILES ${COMPILED_SHADER} DESTINATION "bin/Shaders")

endforeach()

# Install project
install(TARGETS "Renderer"
        DESTINATION "bin")
