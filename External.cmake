

######################################################################
# EXTERNAL LIBRARIES
######################################################################
if (NOT EXTERNAL_INSTALL_LOCATION)
	set(EXTERNAL_INSTALL_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/external)
endif()
if (NOT IS_DIRECTORY ${EXTERNAL_INSTALL_LOCATION})
	file(MAKE_DIRECTORY ${EXTERNAL_INSTALL_LOCATION})
endif()


include(ExternalProject)
# External include directory
include_directories(${EXTERNAL_INSTALL_LOCATION})
add_custom_target(update)



ExternalProject_Add(viltrumDyadic
  GIT_REPOSITORY https://github.com/SergioCE/viltrum_DyadicNets.git
  SOURCE_DIR ${EXTERNAL_INSTALL_LOCATION}/viltrumDyadic
  GIT_TAG main
  UPDATE_DISCONNECTED 1
  STEP_TARGETS update
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
)
add_dependencies(update viltrumDyadic-update)

ExternalProject_Add(viltrum
  GIT_REPOSITORY https://github.com/adolfomunoz/viltrum.git
  SOURCE_DIR ${EXTERNAL_INSTALL_LOCATION}/viltrum
  GIT_TAG main
  UPDATE_DISCONNECTED 1
  STEP_TARGETS update
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
)
add_dependencies(update viltrum-update)




ExternalProject_Add(cimg
  GIT_REPOSITORY https://github.com/dtschump/CImg.git 
  SOURCE_DIR ${EXTERNAL_INSTALL_LOCATION}/CImg
  UPDATE_DISCONNECTED 1
  STEP_TARGETS update
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
)
add_dependencies(update cimg-update)

ExternalProject_Add(cimg-additions
  GIT_REPOSITORY https://github.com/adolfomunoz/cimg_additions.git
  SOURCE_DIR ${EXTERNAL_INSTALL_LOCATION}/cimg_additions
  UPDATE_DISCONNECTED 1
  STEP_TARGETS update
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
)
add_dependencies(update cimg-additions-update)

ExternalProject_Add(eigen
  GIT_REPOSITORY https://github.com/eigenteam/eigen-git-mirror
  SOURCE_DIR ${EXTERNAL_INSTALL_LOCATION}/eigen
  UPDATE_DISCONNECTED 1
  STEP_TARGETS update
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
)
add_dependencies(update eigen-update)

ExternalProject_Add(pbrt
  GIT_REPOSITORY https://github.com/mmp/pbrt-v4.git
  SOURCE_DIR ${EXTERNAL_INSTALL_LOCATION}/pbrt
  UPDATE_DISCONNECTED 1
  STEP_TARGETS update
  #COMMAND 
  BUILD_COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/pbrt_changes/sampler.h ${CMAKE_CURRENT_SOURCE_DIR}/external/pbrt/src/pbrt/base/sampler.h
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/pbrt_changes/samplers.h ${CMAKE_CURRENT_SOURCE_DIR}/external/pbrt/src/pbrt/samplers.h
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/pbrt_changes/integrators.cpp ${CMAKE_CURRENT_SOURCE_DIR}/external/pbrt/src/pbrt/cpu/integrators.cpp
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/pbrt_changes/integrators.h ${CMAKE_CURRENT_SOURCE_DIR}/external/pbrt/src/pbrt/cpu/integrators.h
  COMMAND ${CMAKE_COMMAND} --build .
  #CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
)
add_dependencies(update pbrt-update)


ExternalProject_Add(utf8proc
  GIT_REPOSITORY https://github.com/JuliaStrings/utf8proc.git
  SOURCE_DIR ${EXTERNAL_INSTALL_LOCATION}/utf8proc
  UPDATE_DISCONNECTED 1
  STEP_TARGETS update
  BUILD_COMMAND ""
  #BUILD_COMMAND echo hola 
  #COMMAND cmake --build .
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
)
add_dependencies(update utf8proc-update)





include_directories(${EXTERNAL_INSTALL_LOCATION}/eigen)
include_directories(${EXTERNAL_INSTALL_LOCATION}/pbrt)
include_directories(${EXTERNAL_INSTALL_LOCATION}/CImg)
include_directories(${EXTERNAL_INSTALL_LOCATION}/cimg_additions)