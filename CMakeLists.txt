cmake_minimum_required(VERSION 2.8)
project(mayaAnimCurveMatch)
set(CMD_NAME animCurveMatch)
set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_BUILD_TYPE RELEASE)

# Maya SDK
set(MAYA_ROOT "/usr/autodesk/maya2016")
set(MAYA_INCLUDE ${MAYA_ROOT}/include)
set(MAYA_LIB ${MAYA_ROOT}/lib)

# Lev-Mar
set(LEVMAR_ROOT "/home/davidc/bin/levmar/levmar-2.6")
set(LEVMAR_INCLUDE ${LEVMAR_ROOT})
set(LEVMAR_LIB ${LEVMAR_ROOT})

# Utilities
set(UTILS_INCLUDE "/home/davidc/dev/cppTests/utilities")

set(UTILITIES_FILES
        include/utilities/debugUtils.h)

#
set(SOURCE_FILES
        ${UTILITIES_FILES}
        include/animCurveMatchCmd.h
        include/animCurveMatchUtils.h
        src/animCurveMatchCmd.cpp
        src/animCurveMatchMain.cpp)

include_directories(
        include
        ${UTILS_INCLUDE}
        ${LEVMAR_INCLUDE}
        ${MAYA_INCLUDE}
)

link_directories(
        ${LIBRARY_OUTPUT_PATH}
        ${LEVMAR_LIB}
        ${MAYA_LIB}
)

# 'animCurveMatchCmd' maya plugin library
add_library(${CMD_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${CMD_NAME}
        OpenMaya
        OpenMayaAnim
        Foundation
        levmar
        lapack
        blas
        m)
set_target_properties(${CMD_NAME} PROPERTIES
        PREFIX "" # no 'lib' prefix to .so files
        )