# CMake entry point.

cmake_minimum_required (VERSION 2.6)
project (minecrouft)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(./lib/glfw)
add_subdirectory(./lib/assimp)

set(MY_HEADER_FILES
	"src/Camera.hpp"
	"src/Window.hpp"
	"src/Shader.hpp"
	"src/Mesh.hpp"
	"src/Keyboard.hpp"
)

set(MY_SOURCE_FILES
	"src/main.cpp"
	"src/Camera.cpp"
	"src/Shader.cpp"
	"src/Mesh.cpp"
	"src/Keyboard.cpp"
	"src/glad.c"
)

include_directories(
	${OPENGL_INCLUDE_DIR}
	${GLEW_INCLUDE_DIRS}
	./lib/assimp/include
	"./inc"
)

add_executable(minecrouft
	${MY_HEADER_FILES}
	${MY_SOURCE_FILES}
)
target_link_libraries(minecrouft
	${OPENGL_gl_LIBRARY} # filled by "find_package(OpenGL REQUIRED)"
	glfw
	${GLEW_LIBRARIES}
)