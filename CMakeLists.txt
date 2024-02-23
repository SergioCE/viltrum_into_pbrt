project(VILT_PBRT)
cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include(External)
include(Paths)
include(Compiler)

# Find the OpenEXR package
find_package(OpenEXR REQUIRED)
# Find the zlib package
find_package(ZLIB REQUIRED)
#Find the TBB package
find_package(TBB REQUIRED)

include_directories(external/pbrt/src external/pbrt/src/ext/openvdb/nanovdb ${OpenEXR_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})

set(link_libs ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/libsky_lib.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/libpbrt_lib.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/libdeflate/libdeflate.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/utf8proc/libutf8proc.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/glfw/src/libglfw3.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/glad/libglad.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/ptex/src/ptex/libPtex.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/libflip_lib.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/double-conversion/libdouble-conversion.a 
 ${OpenEXR_LIBRARIES} ${ZLIB_LIBRARIES} OpenEXR::IlmImf TBB::tbb)


#add_executable(render_old main/render_old.cc )
#target_compile_options(render_old PRIVATE -w)
#target_link_libraries(render_old PRIVATE ${link_libs})

#add_executable(render_new main/render_new.cc )
#target_compile_options(render_new PRIVATE -w)
#target_link_libraries(render_new PRIVATE ${link_libs})

add_executable(pbrtPrueba main/pbrtMain.cc )
target_compile_options(pbrtPrueba PRIVATE -w)
target_link_libraries(pbrtPrueba PRIVATE ${link_libs})

add_executable(render_monte_carlo main/render_monte_carlo.cc )
target_compile_options(render_monte_carlo PRIVATE -w)
target_link_libraries(render_monte_carlo PRIVATE ${link_libs})

add_executable(render_newton_cotes main/render_newton_cotes.cc )
target_compile_options(render_newton_cotes PRIVATE -w)
target_link_libraries(render_newton_cotes PRIVATE ${link_libs})

add_executable(render_adaptive main/render_adaptive.cc )
target_compile_options(render_adaptive PRIVATE -w)
target_link_libraries(render_adaptive PRIVATE ${link_libs})

add_executable(render_control_variates main/render_control_variates.cc )
target_compile_options(render_control_variates PRIVATE -w)
target_link_libraries(render_control_variates PRIVATE ${link_libs})

#add_executable(inverse main/inverse.cc
#        external/pbrt/src/pbrt/cpu/integrators.cpp)
