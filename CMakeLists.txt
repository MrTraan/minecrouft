# CMake entry point.

cmake_minimum_required (VERSION 3.8)

project (minecrouft)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set (CMAKE_CXX_STANDARD 17)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(lz4 CONFIG REQUIRED)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

add_compile_definitions( TRACY_ENABLE=1 )

set(GLM_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/lib/glm/")

include_directories(
	${GLM_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIRS}
	"./src"
	"./lib"
)

set(MY_SOURCE_FILES
	"src/main.cpp"
	"src/Camera.cpp"
	"src/Shader.cpp"
	"src/Mesh.cpp"
	"src/Chunk.cpp"
	"src/TextureAtlas.cpp"
	"src/ChunkManager.cpp"
	"src/Frustrum.cpp"
    "src/HeightMap.cpp"
	"src/Player.cpp"
	"src/IO.cpp"
	"src/Hud.cpp"
	"src/Guizmo.cpp"
	
	"src/ngLib/console.cpp"
	"src/ngLib/logs.cpp"
	"src/ngLib/nglib.cpp"
	"src/ngLib/sys.cpp"

	"lib/glad.c"
	# imgui
	"./lib/imgui/imgui.cpp"
	"./lib/imgui/imgui_widgets.cpp"
	"./lib/imgui/imgui_demo.cpp"
	"./lib/imgui/imgui_draw.cpp"
	"./lib/imgui/imgui_impl_sdl.cpp"
	"./lib/imgui/imgui_impl_opengl3.cpp"

	"./lib/FastNoise/FastNoise.cpp"

	"./lib/tracy/TracyClient.cpp"
)

## Used for ycm
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(minecrouft
	${MY_SOURCE_FILES})

target_link_libraries(minecrouft PRIVATE
	${OPENGL_gl_LIBRARY} # filled by "find_package(OpenGL REQUIRED)"
	SDL2::SDL2main SDL2::SDL2-static
	lz4::lz4
)

add_custom_command(TARGET minecrouft POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resources" $<TARGET_FILE_DIR:minecrouft>/resources)

