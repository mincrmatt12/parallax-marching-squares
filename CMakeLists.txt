cmake_minimum_required(VERSION 3.10)
project(parallax_square)

add_subdirectory(vendor/noise)
find_package(SFML 2.5 COMPONENTS graphics window REQUIRED)
find_package(OpenGL REQUIRED)
find_package(gl3w REQUIRED)
find_package(glm REQUIRED)

add_compile_options(-g)

add_executable(parallax_square 
	src/main.cpp
	src/noisegen.cpp
	src/renderman.cpp
)

set_property(TARGET parallax_square PROPERTY CXX_STANDARD 17)
target_link_libraries(parallax_square sfml-graphics sfml-window fastnoise OpenGL::GL gl3w glm)
