# CMake entry point.

cmake_minimum_required (VERSION 2.6)
project (minecrouft)

find_package(OpenGL REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(./lib/glfw)


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
	"src/Cube.hpp"
	"src/Chunk.hpp"
	"src/Debug.hpp"

	"./inc/stb_image.h"

	#imgui
	"./src/imgui/imconfig.h"
	"./src/imgui/imgui.h"
	"./src/imgui/imgui_internal.h"
	"./src/imgui/imgui_impl_glfw_gl3.h"
)

set(MY_SOURCE_FILES
	"src/main.cpp"
	"src/Camera.cpp"
	"src/Shader.cpp"
	"src/Mesh.cpp"
	"src/Keyboard.cpp"
	"src/Mouse.cpp"
	"src/Texture.cpp"
	"src/Cube.cpp"
	"src/Chunk.cpp"
	"src/glad.c"

	# imgui
	"./src/imgui/imgui.cpp"
	"./src/imgui/imgui_demo.cpp"
	"./src/imgui/imgui_draw.cpp"
	"./src/imgui/imgui_impl_glfw_gl3.cpp"
)

include_directories(
	${OPENGL_INCLUDE_DIR}
	"./src"
	"./inc"
)

add_executable(minecrouft
	${MY_HEADER_FILES}
	${MY_SOURCE_FILES}
)
target_link_libraries(minecrouft
	${OPENGL_gl_LIBRARY} # filled by "find_package(OpenGL REQUIRED)"
	glfw
)
