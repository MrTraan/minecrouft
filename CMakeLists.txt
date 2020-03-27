# CMake entry point.

cmake_minimum_required (VERSION 2.6)
message("CMake version: ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
project (minecrouft)
set (CMAKE_CXX_STANDARD 14)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(GLM_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/lib/glm/")

add_compile_definitions( TRACY_ENABLE=1 )

set(MY_HEADER_FILES
	"src/Camera.hpp"
	"src/Window.hpp"
	"src/Shader.hpp"
	"src/Mesh.hpp"
	"src/Keyboard.hpp"
	"src/Mouse.hpp"
	"src/Chunk.hpp"
	"src/Debug.hpp"
	"src/TextureAtlas.hpp"
	"src/ChunkManager.hpp"
	"src/Frustrum.hpp"
    "src/HeightMap.hpp"
    "src/constants.hpp"
	"src/Skybox.hpp"
	"src/LuaConsole.hpp"

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
	"src/Chunk.cpp"
	"src/TextureAtlas.cpp"
	"src/ChunkManager.cpp"
	"src/Frustrum.cpp"
    "src/HeightMap.cpp"
	"src/LuaConsole.cpp"

	"src/glad.c"
	# imgui
	"./src/imgui/imgui.cpp"
	"./src/imgui/imgui_demo.cpp"
	"./src/imgui/imgui_draw.cpp"
	"./src/imgui/imgui_impl_glfw_gl3.cpp"

	"./lib/FastNoise/FastNoise.cpp"

	"./lib/tracy/TracyClient.cpp"
)

## Used for ycm
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
	${OPENGL_INCLUDE_DIR}
	${GLM_INCLUDE_DIRS}
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

add_custom_command(TARGET minecrouft POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resources" $<TARGET_FILE_DIR:minecrouft>/resources)

