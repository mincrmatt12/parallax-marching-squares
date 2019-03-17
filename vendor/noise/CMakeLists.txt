cmake_minimum_required(VERSION 3.10)
project(fastnoise_simd)

file(GLOB srcs *.cpp)

add_library(fastnoise ${srcs})
target_include_directories(fastnoise PUBLIC .)

set_property(TARGET fastnoise PROPERTY CXX_STANDARD 11)

# Compile options

set_source_files_properties(FastNoiseSIMD_avx2.cpp PROPERTIES COMPILE_FLAGS -mavx2)
set_source_files_properties(FastNoiseSIMD_avx2.cpp PROPERTIES COMPILE_FLAGS -march=core-avx2)
set_source_files_properties(FastNoiseSIMD_avx512.cpp PROPERTIES COMPILE_FLAGS -mavx512f)
set_source_files_properties(FastNoiseSIMD_sse2.cpp PROPERTIES COMPILE_FLAGS -msse2)
set_source_files_properties(FastNoiseSIMD_sse41.cpp PROPERTIES COMPILE_FLAGS -msse4.1)
