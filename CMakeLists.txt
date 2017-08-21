cmake_minimum_required(VERSION 2.8)
project(mayaAnimCurveMatch)
set(CMD_NAME animCurveMatch)
set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "The build type")

# Maya SDK
set(MAYA_INCLUDE_PATH "/usr/autodesk/maya2016/include" CACHE PATH "Maya include directory")
set(MAYA_LIB_PATH "/usr/autodesk/maya2016/lib" CACHE PATH "Maya library directory")

# Lev-Mar
SET(LEVMAR_INCLUDE_PATH "/usr/local/include" CACHE PATH "Levmar include directory")
set(LEVMAR_LIB_PATH "/usr/local/lib" CACHE PATH "Levmar library directory")

# Source
set(SOURCE_FILES
        include/utilities/debugUtils.h
        include/animCurveMatchCmd.h
        include/animCurveMatchUtils.h
        src/animCurveMatchCmd.cpp
        src/animCurveMatchMain.cpp)

include_directories(
        include
        ${LEVMAR_INCLUDE_PATH}
        ${MAYA_INCLUDE_PATH}
)

link_directories(
        ${LIBRARY_OUTPUT_PATH}
        ${LEVMAR_LIB_PATH}
        ${MAYA_LIB_PATH}
)

# 'animCurveMatchCmd' maya plugin library
add_library(${CMD_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${CMD_NAME}
        OpenMaya
        OpenMayaAnim
        Foundation
        levmar
        m)
set_target_properties(${CMD_NAME} PROPERTIES
        PREFIX "" # no 'lib' prefix to .so files
        )