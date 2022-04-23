project(VILT_PBRT)
cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include(External)
include(Paths)
include(Compiler)

include_directories(external/pbrt/src external/pbrt/src/ext/ external/pbrt/src/ext/openvdb/nanovdb external/pbrt/src/ext/utf8proc
external/pbrt/src/ext/libdeflate external/pbrt/src/ext/filesystem external/pbrt/src/ext/openexr/OpenEXR/IlmImf external/pbrt/build/src/ext/openexr/OpenEXR/config
external/pbrt/src/ext/openexr/IlmBase/Iex external/pbrt/build/src/ext/openexr/IlmBase/config external/pbrt/src/ext/openexr/IlmBase/Imath
external/pbrt/src/ext/openexr/IlmBase/Half external/pbrt/src/ext/double-conversion/)
#add_executable(sphere_image main/sphere_image.cc)
#add_executable(sphere_sampler main/sphere_sampler.cc)



#add_executable(sphere_samplerPbrt main/sphere_sampler.cc)
#target_link_libraries(sphere_samplerPbrt ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/libpbrt_lib.a)

#target_compile_definitions(sphere_samplerPbrt PRIVATE "PBRT_IS_OSX" "PBRT_HAVE_POSIX_MEMALIGN") libilmimf
add_executable(testF main/testF.cc )

add_executable(pbrtPrueba main/pbrtMain.cc )

target_link_libraries(pbrtPrueba ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/libpbrt_lib.a ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/libsky_lib.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/libdeflate/libdeflate.a ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/utf8proc/libutf8proc.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/openexr/OpenEXR/IlmImf/libIlmImf-2_5.a ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/openexr/IlmBase/Half/libHalf-2_5.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/openexr/IlmBase/Iex/libIex-2_5.a  ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/openexr/IlmBase/IlmThread/libIlmThread-2_5.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/openexr/IlmBase/Imath/libImath-2_5.a  ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/double-conversion/libdouble-conversion.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/ptex/src/ptex/libPtex.a  /Library/Developer/CommandLineTools/SDKs/MacOSX12.3.sdk/usr/lib/libz.tbd )


target_link_libraries(testF ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/libpbrt_lib.a ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/libsky_lib.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/libdeflate/libdeflate.a ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/utf8proc/libutf8proc.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/openexr/OpenEXR/IlmImf/libIlmImf-2_5.a ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/openexr/IlmBase/Half/libHalf-2_5.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/openexr/IlmBase/Iex/libIex-2_5.a  ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/openexr/IlmBase/IlmThread/libIlmThread-2_5.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/openexr/IlmBase/Imath/libImath-2_5.a  ${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/double-conversion/libdouble-conversion.a
${CMAKE_CURRENT_SOURCE_DIR}/build/pbrt-prefix/src/pbrt-build/src/ext/ptex/src/ptex/libPtex.a /Library/Developer/CommandLineTools/SDKs/MacOSX12.3.sdk/usr/lib/libz.tbd )
#libIexMath-2_5.a libIlmImfUtil-2_5.a libdouble-conversion.a /Users/sergiocartiel/Desktop/renderInternship/viltrum_into_pbrt/build/pbrt-prefix/src/pbrt-build/src/ext/zlib/libz.a