cmake_minimum_required(VERSION 3.3)
project(RobotRaconteur)

if(EMSCRIPTEN)
# Workaround for settings in Emscripten.cmake
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY "BOTH")
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE "BOTH")
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE "BOTH")
endif()

include(cmake/CMakeMacros.cmake)

include(CheckTypeSize)
check_type_size("long int" SIZEOF_LONG_INT BUILTIN_TYPES_ONLY)

CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/RobotRaconteurCore/include/RobotRaconteur/RobotRaconteurConfig.h" "${CMAKE_BINARY_DIR}/RobotRaconteurConfig.h" COPYONLY)

FILE (STRINGS "${CMAKE_SOURCE_DIR}/RobotRaconteurCore/include/RobotRaconteur/RobotRaconteurConfig.h" RobotRaconteurConfigVersion REGEX "#define +ROBOTRACONTEUR_VERSION_TEXT +\"[0-9]+\\.[0-9]+\\.[0-9]+\"")
IF ("${RobotRaconteurConfigVersion}" STREQUAL "" )
MESSAGE(FATAL_ERROR "Could not read Robot Raconteur version.")
ENDIF()
STRING (REGEX REPLACE "#define ROBOTRACONTEUR_VERSION_TEXT \"([0-9]+\\.[0-9]+\\.[0-9]+)\"" "\\1" RobotRaconteurVersion ${RobotRaconteurConfigVersion})
MESSAGE(STATUS "Robot Raconteur version: ${RobotRaconteurVersion}")

SET(RobotRaconteur_DISABLE_MESSAGE4 TRUE CACHE BOOL "Disable Robot Raconteur message version 3 by default" )
SET(RobotRaconteur_DISABLE_STRINGTABLE TRUE CACHE BOOL "Disable Robot Raconteur string table by default" )
mark_as_advanced(RobotRaconteur_DISABLE_MESSAGE4 RobotRaconteur_DISABLE_STRINGTABLE)

IF(RobotRaconteur_DISABLE_MESSAGE4)
add_definitions(-DROBOTRACONTEUR_DISABLE_MESSAGE4)
ENDIF()
IF(RobotRaconteur_DISABLE_STRINGTABLE)
add_definitions(-DROBOTRACONTEUR_DISABLE_STRINGTABLE)
ENDIF()

#Find boost and get it ready for use
INCLUDE(FindBoost)
#If(NOT DEFINED Boost_USE_STATIC_LIBS)
#IF(RobotRaconteur_USE_SHARED_CORE_LIB)
#SET (Boost_USE_STATIC_LIBS OFF)
#ELSE()
#SET (Boost_USE_STATIC_LIBS ON)
#ENDIF()
#ENDIF()
SET (Boost_USE_MULTITHREADED ON)
SET (Boost_USE_STATIC_RUNTIME OFF)
FIND_PACKAGE(Boost COMPONENTS date_time filesystem system regex chrono random program_options REQUIRED)
include_directories("${CMAKE_SOURCE_DIR}/RobotRaconteurCore/include" ${Boost_INCLUDE_DIRS})
add_definitions(-DBOOST_ALL_NO_LIB)



add_subdirectory(RobotRaconteurCore)
set(RobotRaconteur_CORE_LIBRARY RobotRaconteurCore)

set(BUILD_PYTHON "" CACHE BOOL "Build Python wrappers")
if (BUILD_PYTHON)
add_subdirectory(RobotRaconteurPython)
endif()
