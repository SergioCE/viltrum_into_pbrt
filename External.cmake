

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
add_dependencies(update viltrum)



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
  BUILD_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
)
add_dependencies(update pbrt)




include_directories(${EXTERNAL_INSTALL_LOCATION}/eigen)
include_directories(${EXTERNAL_INSTALL_LOCATION}/pbrt)
include_directories(${EXTERNAL_INSTALL_LOCATION}/CImg)
include_directories(${EXTERNAL_INSTALL_LOCATION}/cimg_additions)