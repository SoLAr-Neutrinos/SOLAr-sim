#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(SOLArUtils LANGUAGES CXX C VERSION 0.3.0)

list(APPEND CMAKE_PREFIX_PATH 
  $ENV{ROOTSYS}
  CMAKE_LIBRARY_OUTPUT_DIRECTORY
)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

if (CMAKE_BUILD_TYPE STREQUAL "Debug") 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
  message(STATUS "DEBUG: Adding '-g' option for gdb debugging")
endif()

if (SLAR_PROFILE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
  message(STATUS "Building solar_sim with gprof profiling feature")
endif()

find_package(ROOT REQUIRED COMPONENTS RIO Core)
include(${ROOT_USE_FILE})

find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})

message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package(SOLAr-sim CONFIG REQUIRED)
if(SOLAr-sim_FOUND)
  message(STATUS "SOLAr-sim found: libraries at ${SOLARSIM_LIB_DIR}")
  list(APPEND CMAKE_PREFIX_PATH ${SOLARSIM_BIN_DIR})
  list(APPEND CMAKE_PREFIX_PATH ${SOLARSIM_EXT_DIR})
endif()

find_package(RapidJSON REQUIRED)
if (RapidJSON_FOUND) 
  message(STATUS "RapidJSON found: include at ${RapidJSON_INCLUDE_DIR}")
endif()

#----------------------------------------------------------------------------
# Define path variables

list(APPEND CMAKE_LIBRARY_PATH ${SOLARSIM_LIB_DIR})
set(SOLAR_UTILS_INC_DIR    "${PROJECT_SOURCE_DIR}/include")
set(SOLAR_UTILS_SRC_DIR    "${PROJECT_SOURCE_DIR}/src")
set(SOLAR_UTILS_SCRIPT_DIR "${PROJECT_SOURCE_DIR}/script")
set(SOLAR_UTILS_BIN_DIR  "${CMAKE_INSTALL_PREFIX}/bin")
set(SOLAR_UTILS_LIB_DIR  "${CMAKE_INSTALL_PREFIX}/lib")
set(SOLAR_UTILS_INC_DEST "${CMAKE_INSTALL_PREFIX}/include")
set(SOLAR_UTILS_INSTALL_CMAKE_DIR "${SOLAR_UTILS_LIB_DIR}/cmake/${PROJECT_NAME}")

include(ExternalProject)
#ExternalProject_Add("progressbar"
  #SOURCE_DIR      ${CMAKE_CURRENT_BINARY_DIR}/progressbar
  #INSTALL_DIR     ${SOLAR_UTILS_INC_DIR}
  #UPDATE_DISCONNECTED 1
  #GIT_REPOSITORY  "https://github.com/dguff/progressbar.git"
  #GIT_TAG         "main"
  #CONFIGURE_COMMAND echo "configuring progressbar"
  #BUILD_COMMAND echo "fetching progressbar"
  #INSTALL_COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/progressbar/include/progressbar.hpp ${SOLAR_UTILS_INC_DIR} )

include_directories(
  ${SOLAR_UTILS_INC_DIR}
  ${SOLARSIM_INCLUDE_DIR}
  ${Geant4_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIRS}
  "${RapidJSON_INCLUDE_DIR}"
  )
if (CLHEP_EXTERNAL) 
  include_directories(${CLHEP_EXTERNAL})
endif()

#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Build project
add_subdirectory(${SOLAR_UTILS_SRC_DIR})
add_subdirectory(${SOLAR_UTILS_SCRIPT_DIR})

#----------------------------------------------------------------------------
# Install instructions
install(DIRECTORY ${SOLAR_UTILS_INC_DIR}
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.hh" PATTERN "*.h" PATTERN "*.hpp"
  )
