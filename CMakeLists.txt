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

include_directories(external/pbrt/src external/pbrt/src/ext/openvdb/nanovdb ${OpenEXR_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})


add_executable(render_old main/render_old.cc )

add_executable(render_new main/render_new.cc )


target_compile_options(render_new PRIVATE
    -w  # Disable all warnings
)

target_compile_options(render_old PRIVATE
    -w  # Disable all warnings
)

# Find the OpenEXR package
find_package(OpenEXR REQUIRED)

# Find the zlib package
find_package(ZLIB REQUIRED)

#Find the TBB package
find_package(TBB REQUIRED)



target_link_libraries(render_new ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/libsky_lib.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/libpbrt_lib.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/libdeflate/libdeflate.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/utf8proc/libutf8proc.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/glfw/src/libglfw3.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/glad/libglad.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/ptex/src/ptex/libPtex.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/libflip_lib.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/double-conversion/libdouble-conversion.a 
 ${OpenEXR_LIBRARIES} ${ZLIB_LIBRARIES} OpenEXR::IlmImf TBB::tbb)


target_link_libraries(render_old ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/libsky_lib.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/libpbrt_lib.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/libdeflate/libdeflate.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/utf8proc/libutf8proc.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/glfw/src/libglfw3.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/glad/libglad.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/ptex/src/ptex/libPtex.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/libflip_lib.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/double-conversion/libdouble-conversion.a 
 ${OpenEXR_LIBRARIES} ${ZLIB_LIBRARIES} OpenEXR::IlmImf TBB::tbb)