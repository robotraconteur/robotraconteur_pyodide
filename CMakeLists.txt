cmake_minimum_required(VERSION 3.3)
project(RobotRaconteur-pyodide)

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

file(STRINGS "${CMAKE_SOURCE_DIR}/RobotRaconteurCore/include/RobotRaconteur/RobotRaconteurConfig.h"
     ROBOTRACONTEUR_CONFIG_VERSION REGEX "#define +ROBOTRACONTEUR_VERSION_TEXT +\"[0-9]+\\.[0-9]+\\.[0-9]+\"")
if("${ROBOTRACONTEUR_CONFIG_VERSION}" STREQUAL "")
    message(FATAL_ERROR "Could not read Robot Raconteur version.")
endif()
string(REGEX REPLACE "#define ROBOTRACONTEUR_VERSION_TEXT \"([0-9]+\\.[0-9]+\\.[0-9]+)\"" "\\1" ROBOTRACONTEUR_VERSION
                     ${ROBOTRACONTEUR_CONFIG_VERSION})
message(STATUS "Robot Raconteur version: ${ROBOTRACONTEUR_VERSION}")

if(NOT ROBOTRACONTEUR_VERSION_SEMVER)
    set(ROBOTRACONTEUR_VERSION_SEMVER ${ROBOTRACONTEUR_VERSION})
endif()

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

if(BUILD_PYTHON)
    if(USE_PREGENERATED_SOURCE)
        set(PREGENERATED_SOURCE_DIR ${CMAKE_SOURCE_DIR}/generated_src CACHE PATH "generated_src directory location")
        if(NOT EXISTS ${PREGENERATED_SOURCE_DIR}/Python/swigwordsize32/RobotRaconteurPythonPYTHON_wrap.cxx)
            message(FATAL_ERROR "Invalid PREGENERATED_SOURCE_DIR")
        endif()
    else()
        include(${CMAKE_SOURCE_DIR}/cmake/package_source_swig.cmake)
    endif()
endif()

list(APPEND CPACK_SOURCE_IGNORE_FILES "/\.git/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\.vs/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\.vscode/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\.github/")
# list(APPEND CPACK_SOURCE_IGNORE_FILES "/build/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "^${CMAKE_SOURCE_DIR}/build[\\\\-_/]")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\.cache")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\.gitignore")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\.travis.yml")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\.clang-format")

set(CPACK_PACKAGE_VERSION ${ROBOTRACONTEUR_VERSION_SEMVER})

set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CMAKE_SOURCE_DIR};/;${CMAKE_BINARY_DIR}/generated_src;/generated_src")
set(CPACK_SOURCE_TBZ2 OFF CACHE BOOL "" FORCE)
set(CPACK_SOURCE_TXZ OFF CACHE BOOL "" FORCE)
set(CPACK_SOURCE_TZ OFF CACHE BOOL "" FORCE)
if(NOT WIN32)
    set(CPACK_SOURCE_TGZ ON CACHE BOOL "" FORCE)
endif()
set(CPACK_SOURCE_ZIP ON CACHE BOOL "" FORCE)

include(CPack)
