# Copyright (c) 2017,2018 Regents of the University of Minnesota.
# All rights reserved.
#
# Original Author(s) of this File: 
#   Daniel Keefe, Jan 2018, University of Minnesota -- This build system is inspired by
#   the AUTOBUILD_DEPENDENCY approach I developed for the MinVR Open Source project.  
#   Tom Sgouros (Brown Univ.) and Dan Orban (Univ. of Minn.) also contributed to earlier 
#   versions of MinVR's build system and the use of cmake in other research software used
#   in the IV/LAB.  So, this is also at least loosly inspired by their contributions.  
#   See minvr.org for a full list of contributors to that project.
#  
# Author(s) of Significant Updates/Modifications to the File:
#   ...




#### BASIC PROJECT SETUP ####

# Depends on cmake 3.9+ for its more advanced FindOpenGL.cmake module
cmake_minimum_required (VERSION 3.9)

# Search path for cmake support scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

# Version is <major>.<minor>.<patch>
cmake_policy(SET CMP0048 NEW)  # Required for setting version number as part of the project() command
project(ForceServer VERSION 1.0.0)

# Organize folder structure for Xcode, Visual Studio, etc.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# For better formatting of output while configuring with cmake
include(MessageMacros)

# If you "autobuild" external dependencies as part of the cmake configure process, then they will be
# downloaded to this directory.
set(AUTOBUILD_DOWNLOAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external" CACHE STRING "Location for external project sources downloaded by AutoBuild.")


set(CMAKE_INSTALL_MESSAGE ALWAYS)

include(AutoBuild)
AutoBuild_init()


#### ADJUST NAMES OF OUTPUT FILES BASED ON BUILD TYPE ####

h1("CONFIGURING BUILD TYPES")

# To distinguish between debugging, release, and other builds, we'll
# add a postfix to the name of the lib or exe that we generate.

set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_RELWITHDEBINFO_POSTFIX "rd")
set(CMAKE_MINSIZEREL_POSTFIX "s")

if (CMAKE_BUILD_TYPE MATCHES "Release")
  set(CMAKE_BUILD_POSTFIX "${CMAKE_RELEASE_POSTFIX}")
elseif (CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
  set(CMAKE_BUILD_POSTFIX "${CMAKE_MINSIZEREL_POSTFIX}")
elseif (CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
  set(CMAKE_BUILD_POSTFIX "${CMAKE_RELWITHDEBINFO_POSTFIX}")
elseif (CMAKE_BUILD_TYPE MATCHES "Debug")
  set(CMAKE_BUILD_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
else()
  set(CMAKE_BUILD_POSTFIX "")
endif()




#### SETUP DIRECTORIES FOR BUILDING AND INSTALLING ####

h1("CONFIGURING BUILD/INSTALL LOCATIONS")

h2("Configuring build directories.")

message(STATUS "Server, tests, other binaries: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")


h2("Configuring installation directories.")
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE )
endif()

# When we autobuild dependencies, we install them in CMAKE_INSTALL_PREFIX, and then look for them
# there using find_package().  So, this makes sure the install prefix is in the path that 
# find_package() searches.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_INSTALL_PREFIX})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})

# These are relative to ${CMAKE_INSTALL_PREFIX}
set(INSTALL_BIN_DEST           bin)

message(STATUS "Server, tests, other binaries: ${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DEST}")



#### SET ANY COMPILER FLAGS ####

h1("CONFIGURING COMPLIER FLAGS")



message(STATUS "Building for " ${CMAKE_SYSTEM_NAME} ".")
message(STATUS "Compiler supported features = ${CMAKE_CXX_COMPILE_FEATURES}")


# Linux specific
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DLINUX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()


# Apple specific
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DOSX)

  # Since for some stupid reason Apple has now deprecated OpenGL
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")
  add_definitions(-DGL_SILENCE_DEPRECATION)

  # RPATH settings, see https://cmake.org/Wiki/CMake_RPATH_handling
  set(CMAKE_MACOSX_RPATH ON)

  # use, i.e. don't skip the full RPATH for the build tree
  SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

  # when building, don't use the install RPATH already
  # (but later on when installing)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

  # add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

  # the RPATH to be used when installing, but only if it's not a system directory
  LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
  IF("${isSystemDir}" STREQUAL "-1")
     SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  ENDIF("${isSystemDir}" STREQUAL "-1")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()


# Windows specific
if (WIN32)
  add_definitions(-DWIN32)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")
endif()




#### DEFINE BUILD TARGETS, LIKE SERVER, TEST PROGRAMS, ETC. ####

h1("CONFIGURING PROGRAMS AND DATA")

h2("Configuring the server.")
message(STATUS "Adding server_src to the build.")
include(server_src/CMakeLists.txt)

h2("Cofiguring test programs.")
message(STATUS "Adding test program to the build.")
add_subdirectory(example_client)


AutoBuild_check_status()

