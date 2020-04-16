# CMake entry point.

cmake_minimum_required (VERSION 3.8)

project (minecrouft)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd26812")

set (CMAKE_CXX_STANDARD 20)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(lz4 REQUIRED)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

if ( ENABLE_PROFILING )
	add_compile_definitions( TRACY_ENABLE=1 )
endif( ENABLE_PROFILING )

set(GLM_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/lib/glm/")

## Used for ycm
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(minecrouft
	"src/main.cpp"
	"src/Camera.cpp"
	"src/Shader.cpp"
	"src/Mesh.cpp"
	"src/Chunk.cpp"
	"src/Block.cpp"
	"src/TextureAtlas.cpp"
	"src/ChunkManager.cpp"
	"src/Frustrum.cpp"
    "src/HeightMap.cpp"
	"src/Player.cpp"
	"src/IO.cpp"
	"src/Hud.cpp"
	"src/Guizmo.cpp"
	"src/Skybox.cpp"
	
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

target_include_directories(minecrouft
PRIVATE
	"./src"
SYSTEM
	${GLM_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIRS}
	"./lib"

)

include_directories(
	"./src"
)


target_link_libraries(minecrouft PRIVATE
	${OPENGL_gl_LIBRARY} # filled by "find_package(OpenGL REQUIRED)"
	SDL2::SDL2main SDL2::SDL2-static
	lz4::lz4
)

add_custom_command(TARGET minecrouft POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resources" $<TARGET_FILE_DIR:minecrouft>/resources)

