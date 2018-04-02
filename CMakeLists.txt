# CMake entry point.

cmake_minimum_required (VERSION 2.6)
project (minecrouft)

find_package(OpenGL REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(./lib/glfw)

# list of files for which we add a copy rule
set(data_SHADOW 
	./shaders/vertex.glsl
	./shaders/fragment.glsl
	./resources/dirt.png
)

# we don't want to copy if we're building in the source dir
if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    foreach(item IN LISTS data_SHADOW)
        message(STATUS ${item})
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${item}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${item}" "${CMAKE_CURRENT_BINARY_DIR}/${item}"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${item}"
        )
    endforeach()
endif()

# files are only copied if a target depends on them
add_custom_target(data-target ALL DEPENDS "${data_SHADOW}")

## glm
find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

set(MY_HEADER_FILES
	"src/Camera.hpp"
	"src/Window.hpp"
	"src/Shader.hpp"
	"src/Mesh.hpp"
	"src/Keyboard.hpp"
	"src/Texture.hpp"
	"src/Mouse.hpp"
	"src/Chunk.hpp"
	"src/Debug.hpp"
	"src/TextureManager.hpp"
	"src/ChunkManager.hpp"
	"src/Frustrum.hpp"

	"./inc/stb_image.h"

	#imgui
	"./src/imgui/imconfig.h"
	"./src/imgui/imgui.h"
	"./src/imgui/imgui_internal.h"
	"./src/imgui/imgui_impl_glfw_gl3.h"

	"./lib/FastNoise/FastNoise.h"
)

set(MY_SOURCE_FILES
	"src/main.cpp"
	"src/Camera.cpp"
	"src/Shader.cpp"
	"src/Mesh.cpp"
	"src/Keyboard.cpp"
	"src/Mouse.cpp"
	"src/Texture.cpp"
	"src/Chunk.cpp"
	"src/TextureManager.cpp"
	"src/ChunkManager.cpp"
	"src/glad.c"
	"src/Frustrum.cpp"

	# imgui
	"./src/imgui/imgui.cpp"
	"./src/imgui/imgui_demo.cpp"
	"./src/imgui/imgui_draw.cpp"
	"./src/imgui/imgui_impl_glfw_gl3.cpp"

	"./lib/FastNoise/FastNoise.cpp"
)

## Used for ycm
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
	${OPENGL_INCLUDE_DIR}
	"./src"
	"./inc"
	"./lib"
)

add_executable(minecrouft
	${MY_HEADER_FILES}
	${MY_SOURCE_FILES}
)
target_link_libraries(minecrouft
	${OPENGL_gl_LIBRARY} # filled by "find_package(OpenGL REQUIRED)"
	glfw
)
