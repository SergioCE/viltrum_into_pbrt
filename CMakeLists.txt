project(VILT_PBRT)
cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include(External)
include(Paths)
include(Compiler)

#add_executable(sphere_image main/sphere_image.cc)
add_executable(sphere_sampler main/sphere_sampler.cc)
