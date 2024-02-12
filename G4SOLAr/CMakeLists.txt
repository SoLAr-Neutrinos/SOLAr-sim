#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(G4SOLAr)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS}
  CMAKE_LIBRARY_OUTPUT_DIRECTORY)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
if (CMAKE_BUILD_TYPE STREQUAL "Debug") 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
  message(STATUS "DEBUG: Adding '-g' option for gdb debugging")
endif()

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

find_package(ROOT REQUIRED COMPONENTS RIO Core)

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
include(${Geant4_USE_FILE})
include(${ROOT_USE_FILE})

message(STATUS "Geant4 GDML SUPPORT: ${Geant4_gdml_FOUND}")

#----------------------------------------------------------------------------
# Setup Projects PATHS
set(G4SOLAR_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(G4SOLAR_SRC_DIR     "${PROJECT_SOURCE_DIR}/src"    )
if (NOT G4SOLAR_EXT_DIR)
  set(G4SOLAR_EXT_DIR   "${PROJECT_SOURCE_DIR}/extern/install" )
endif()
set(G4SOLAR_LIB_DIR     "${CMAKE_INSTALL_PREFIX}/lib"  )
set(G4SOLAR_BIN_DIR     "${CMAKE_INSTALL_PREFIX}"      )
set(G4SOLAR_ANALSYS_DIR "${PROJECT_SOURCE_DIR}/../SOLArAnalysis")
set(G4SOLAR_INSTALL_CMAKE_DIR "${G4SOLAR_LIB_DIR}/cmake/${PROJECT_NAME}")

#----------------------------------------------------------------------------
# Find external dependencies of G4SOLAr.
# If not already installed cd into the G4SOLAR/extern directory and execute the
# cmakelists in there
#
# 1. RapidJSON
message(STATUS "G4SOLAR_EXT_DIR set to ${G4SOLAR_EXT_DIR}")
include(ExternalProject)
if(NOT RAPIDJSON_DIR)
  set(RAPIDJSON_DIR "${G4SOLAR_EXT_DIR}/rapidjson")
endif()
find_package(RapidJSON REQUIRED PATHS ${RAPIDJSON_DIR} NO_DEFAULT_PATH)

if(NOT BXDECAY0_DIR)
  set(BXDECAY0_DIR "${G4SOLAR_EXT_DIR}/bxdecay0")
endif()
find_package(BxDecay0 REQUIRED COMPONENTS Geant4 CONFIG PATHS ${BXDECAY0_DIR} NO_DEFAULT_PATH)

if(BxDecay0_FOUND)
  message(STATUS "BxDecay0 found: include at ${BxDecay0_INCLUDE_DIR}")
  list(APPEND CMAKE_PREFIX_PATH ${BXDECAY0_DIR})
endif()
if (RapidJSON_FOUND)
  message(STATUS "RapidJSON found: include at ${RapidJSON_INCLUDE_DIR}")
endif()

if(NOT MARLEY_DIR)
  set(MARLEY_DIR "${G4SOLAR_EXT_DIR}/marley")
endif()

find_library(MARLEY
  NAMES MARLEY
  PATHS "${MARLEY_DIR}/build"
  REQUIRED
  NO_DEFAULT_PATH
  )
find_library(MARLEY_ROOT
  NAMES MARLEY_ROOT
  PATHS "${MARLEY_DIR}/build"
  REQUIRED
  NO_DEFAULT_PATH
  )

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${G4SOLAR_INCLUDE_DIR}
                    ${G4SOLAR_INCLUDE_DIR}/material
                    ${G4SOLAR_INCLUDE_DIR}/detector
                    ${G4SOLAR_INCLUDE_DIR}/detector/TPC
                    ${G4SOLAR_INCLUDE_DIR}/detector/SuperCell
                    ${G4SOLAR_INCLUDE_DIR}/detector/Anode
                    ${G4SOLAR_INCLUDE_DIR}/config
                    ${G4SOLAR_INCLUDE_DIR}/event
                    ${G4SOLAR_INCLUDE_DIR}/physics
                    ${BxDecay0_INCLUDE_DIR}
                    ${MARLEY_DIR}/include
                    ${RapidJSON_INCLUDE_DIR}/..
                    ${Geant4_INCLUDE_DIR}
                    ${ROOT_INCLUDE_DIRS}
)

if (CLHEP_EXTERNAL) 
  include_directories(${CLHEP_EXTERNAL})
endif()

file(GLOB sources   ${G4SOLAR_SRC_DIR}/*.cc
                    ${G4SOLAR_SRC_DIR}/event/*.cc
                    ${G4SOLAR_SRC_DIR}/config/*.cc
                    ${G4SOLAR_SRC_DIR}/material/*.cc
                    ${G4SOLAR_SRC_DIR}/physics/*.cc
                    ${G4SOLAR_SRC_DIR}/detector/*.cc
                    ${G4SOLAR_SRC_DIR}/detector/TPC/*.cc
                    ${G4SOLAR_SRC_DIR}/detector/SuperCell/*.cc
                    ${G4SOLAR_SRC_DIR}/detector/Anode/*.cc
)

file(GLOB headers   ${G4SOLAR_INCLUDE_DIR}/*.hh
                    ${G4SOLAR_INCLUDE_DIR}/event/*.hh
                    ${G4SOLAR_INCLUDE_DIR}/physics/*.hh
                    ${G4SOLAR_INCLUDE_DIR}/config/*.hh
                    ${G4SOLAR_INCLUDE_DIR}/material/*.hh
                    ${G4SOLAR_INCLUDE_DIR}/detector/*.hh
                    ${G4SOLAR_INCLUDE_DIR}/detector/TPC/*.hh
                    ${G4SOLAR_INCLUDE_DIR}/detector/SuperCell/*.hh
                    ${G4SOLAR_INCLUDE_DIR}/detector/Anode/*.hh
                    )

add_definitions(${ROOT_CXX_FLAGS})

#----------------------------------------------------------------------------
# Configure local paths
configure_file(SLArUserPath.hh.in ${G4SOLAR_INCLUDE_DIR}/SLArUserPath.hh @ONLY)

#----------------------------------------------------------------------------
# Configure rootlogon
configure_file(${G4SOLAR_INCLUDE_DIR}/rootlogon.C.in ${G4SOLAR_ANALSYS_DIR}/rootlogon.C @ONLY)

#----------------------------------------------------------------------------
# Generate SLArEvent and SLArConfig libraries and dictionaries
add_subdirectory(${G4SOLAR_SRC_DIR}/config)
add_subdirectory(${G4SOLAR_SRC_DIR}/event)
add_subdirectory(${G4SOLAR_SRC_DIR}/physics)

#----------------------------------------------------------------------------
# configure package config
include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/G4SOLArConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/G4SOLArConfig.cmake
  INSTALL_DESTINATION "${G4SOLAR_INSTALL_CMAKE_DIR}"
  PATH_VARS G4SOLAR_LIB_DIR G4SOLAR_BIN_DIR G4SOLAR_INCLUDE_DIR G4SOLAR_INSTALL_CMAKE_DIR
  )

# extract git branch and commit hash
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/G4SOLArConfig.cmake
  DESTINATION ${G4SOLAR_INSTALL_CMAKE_DIR})

# configure package config
install(EXPORT G4SOLArTargets
  #FILE   ${CMAKE_CURRENT_BINARY_DIR}/cmake/G4SOLArConfig.cmake
  DESTINATION "${G4SOLAR_INSTALL_CMAKE_DIR}"
  NAMESPACE G4SOLAr::
)


# extract git branch and commit hash
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)


#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#

add_executable(solar_sim solar_sim.cc ${sources} ${headers})
target_link_libraries(solar_sim ${Geant4_LIBRARIES})
target_link_libraries(solar_sim ${ROOT_LIBRARIES})
target_link_libraries(solar_sim SLArMCEvent)
target_link_libraries(solar_sim SLArScintillation)
target_link_libraries(solar_sim SLArReadoutSystemConfig)
target_link_libraries(solar_sim BxDecay0::BxDecay0 BxDecay0::BxDecay0_Geant4)
target_link_libraries(solar_sim ${MARLEY} ${MARLEY_ROOT})
target_compile_definitions(solar_sim 
  PUBLIC 
  $<$<CONFIG:Debug>:SLAR_DEBUG>
  $<$<STREQUAL:${Geant4_gdml_FOUND},ON>:SLAR_GDML>
  $<$<STREQUAL:${SLAR_EXTERNAL},ON>:SLAR_EXTERNAL>
  SLAR_EXTERNAL_PARTICLE="${SLAR_EXTERNAL_PARTICLE}"
  PRIVATE
  "-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\""
  )

list(APPEND G4SOLAR_RPATH
  "${G4SOLAR_LIB_DIR}"
  "${BXDECAY0_DIR}/lib"
  "${BXDECAY0_DIR}/lib64"
  "${MARLEY_DIR}/build")
set_target_properties(solar_sim PROPERTIES
  INSTALL_RPATH "${G4SOLAR_RPATH}"
  BUILD_WITH_INSTALL_RPATH 1
  )
install(TARGETS solar_sim
  LIBRARY DESTINATION ${G4SOLAR_BIN_DIR}
  RUNTIME DESTINATION ${G4SOLAR_BIN_DIR}
  )
#----------------------------------------------------------------------------
# Copy all assets (geometry, materials, cfg) to the install directory.
# This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.

install(DIRECTORY ${PROJECT_SOURCE_DIR}/assets DESTINATION ${G4SOLAR_BIN_DIR})


