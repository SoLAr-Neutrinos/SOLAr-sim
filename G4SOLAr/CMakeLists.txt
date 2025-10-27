#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(SOLAr-sim LANGUAGES CXX C VERSION 0.3.0)

message(STATUS "Building ${PROJECT_NAME} version ${PROJECT_VERSION}")

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS}
  CMAKE_LIBRARY_OUTPUT_DIRECTORY)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

#----------------------------------------------------------------------------
# Define few SoLAr-sim options
option(SLAR_PROFILE "Enbale profiling with gprof" OFF)
option(SLAR_EXTERNAL "Build code with special external background features" OFF)
option(SLAR_EXTERNAL_PARTICLE "Define external background particle of interest" OFF)
option(SLAR_CRY_INTERFACE "Build interface to CRY cosmic shower generator" OFF)
option(SLAR_RADSRC_INTERFACE "Build interface to RadSrc composite gamma spectrum generator" OFF)
option(SLAR_USE_G4CASCADE "Use G4CASCADE for hadronic interactions" ON)

#----------------------------------------------------------------------------
# Setup Projects PATHS
set(SOLARSIM_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(SOLARSIM_SRC_DIR     "${PROJECT_SOURCE_DIR}/src"    )
if (NOT SOLARSIM_EXT_DIR)
  set(SOLARSIM_EXT_DIR   "${PROJECT_SOURCE_DIR}/extern/install" )
endif()
set(SOLARSIM_LIB_DIR     "${CMAKE_INSTALL_PREFIX}/lib")
set(SOLARSIM_BIN_DIR     "${CMAKE_INSTALL_PREFIX}/bin")
set(SOLARSIM_INC_DEST    "${CMAKE_INSTALL_PREFIX}/include")
set(SOLARSIM_ASSETS_DIR "${CMAKE_INSTALL_PREFIX}/assets")
set(SOLARSIM_ANALSYS_DIR "${PROJECT_SOURCE_DIR}/../SOLArAnalysis")
set(SOLARSIM_INSTALL_CMAKE_DIR "${SOLARSIM_LIB_DIR}/cmake/${PROJECT_NAME}")

# Debugging module setup
include( ${PROJECT_SOURCE_DIR}/cmake/SOLArDebugConfig.cmake )
configure_debug_system()


#----------------------------------------------------------------------------
# Enable profiling with gprof
if (SLAR_PROFILE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
  message(STATUS "Building solar_sim with gprof profiling feature")
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

if (DEFINED ENV{CRYHOME}) 
  set(SLAR_CRY_INTERFACE "ON")
  message(STATUS "Found CRY package at $ENV{CRYHOME}. Building CRY interface classes")
  set(CRY_INCLUDE_DIR $ENV{CRYHOME}/src)
  set(CRY_LIBRARIES $ENV{CRYHOME}/lib/libCRY.a)
endif()

if (DEFINED ENV{RADSRC_HOME})
  set(SLAR_RADSRC_INTERFACE "ON")
  message(STATUS "Found RadSrc package at $ENV{RADSRC_HOME}. Building RadSrc interface classes")
  set(RADSRC_INCLUDE_DIR $ENV{RADSRC_HOME}/src/libradsrc)
  set(RADSRC_LIBRARIES $ENV{RADSRC_HOME}/lib/libradsrc.a)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
include(${Geant4_USE_FILE})
include(${ROOT_USE_FILE})

message(STATUS "Geant4 GDML SUPPORT: ${Geant4_gdml_FOUND}")

#----------------------------------------------------------------------------
# Find external dependencies of G4SOLAr.
# If not already installed cd into the SOLARSIM/extern directory and execute the
# cmakelists in there

# 1. RapidJSON
message(STATUS "SOLARSIM_EXT_DIR set to ${SOLARSIM_EXT_DIR}")
include(ExternalProject)
if(NOT RAPIDJSON_DIR)
  set(RAPIDJSON_DIR "${SOLARSIM_EXT_DIR}/rapidjson")
endif()
find_package(RapidJSON REQUIRED PATHS ${RAPIDJSON_DIR} NO_DEFAULT_PATH)

# 2. BxDecay0
if(NOT BXDECAY0_DIR)
  set(BXDECAY0_DIR "${SOLARSIM_EXT_DIR}/bxdecay0")
endif()
find_package(BxDecay0 REQUIRED COMPONENTS Geant4 CONFIG PATHS ${BXDECAY0_DIR} NO_DEFAULT_PATH)

if(BxDecay0_FOUND)
  message(STATUS "BxDecay0 found: include at ${BxDecay0_INCLUDE_DIR}")
  list(APPEND CMAKE_PREFIX_PATH ${BXDECAY0_DIR})
endif()
if (RapidJSON_FOUND)
  message(STATUS "RapidJSON found: include at ${RapidJSON_INCLUDE_DIR}")
endif()

# 3. MARLEY
if(NOT MARLEY_DIR)
  set(MARLEY_DIR "${SOLARSIM_EXT_DIR}/marley")
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

# 4. G4CASCADE
if (SLAR_USE_G4CASCADE)
  include( ${PROJECT_SOURCE_DIR}/cmake/G4CASCADE_setup.cmake )
endif()

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(
  ${BxDecay0_INCLUDE_DIR}
  ${MARLEY_DIR}/include
  ${RapidJSON_INCLUDE_DIR}
  ${Geant4_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIRS}
)

if (CLHEP_EXTERNAL) 
  include_directories(${CLHEP_EXTERNAL})
endif()

add_definitions(${ROOT_CXX_FLAGS})

#----------------------------------------------------------------------------
# Configure version and local paths
configure_file(SLArVersion.hh.in ${SOLARSIM_INCLUDE_DIR}/SLArVersion.hh @ONLY)
configure_file(SLArUserPath.hh.in ${SOLARSIM_INCLUDE_DIR}/SLArUserPath.hh @ONLY)

#----------------------------------------------------------------------------
# Configure rootlogon
configure_file(${PROJECT_SOURCE_DIR}/rootlogon.C.in ${SOLARSIM_BIN_DIR}/rootlogon.C @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/rootrc.in ${CMAKE_INSTALL_PREFIX}/.rootrc @ONLY)

#----------------------------------------------------------------------------
# Generate SLArEvent and SLArConfig libraries and dictionaries
set(solarsim_sources "")
set(solarsim_headers "")

add_subdirectory(${SOLARSIM_SRC_DIR})
add_subdirectory(${SOLARSIM_SRC_DIR}/config)
add_subdirectory(${SOLARSIM_SRC_DIR}/event)
add_subdirectory(${SOLARSIM_SRC_DIR}/action)
add_subdirectory(${SOLARSIM_SRC_DIR}/geo)
add_subdirectory(${SOLARSIM_SRC_DIR}/gen)
add_subdirectory(${SOLARSIM_SRC_DIR}/analysis)
add_subdirectory(${SOLARSIM_SRC_DIR}/physics)

# configure package config
install(EXPORT "${PROJECT_NAME}Targets"
  DESTINATION "${SOLARSIM_INSTALL_CMAKE_DIR}"
  NAMESPACE SOLARSIM::
)

# extract git branch and commit hash
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(solar_sim solar_sim.cc ${solarsim_sources} ${solarsim_headers})

# Link the libraries
target_link_libraries(solar_sim PRIVATE 
  ${Geant4_LIBRARIES} 
  ${ROOT_LIBRARIES}
  SLArMCTruth
  SLArMCEventReadout
  SLArGenRecords
  SLArReadoutSystemConfig
  BxDecay0::BxDecay0 BxDecay0::BxDecay0_Geant4
  ${MARLEY} ${MARLEY_ROOT}
)
if (SLAR_CRY_INTERFACE)
  target_link_libraries(solar_sim PRIVATE ${CRY_LIBRARIES})
endif()
if (SLAR_RADSRC_INTERFACE) 
  target_link_libraries(solar_sim PRIVATE ${RADSRC_LIBRARIES})
endif()

# include directories
target_include_directories(solar_sim PUBLIC
  $<BUILD_INTERFACE:${SOLARSIM_INCLUDE_DIR}>
  $<BUILD_INTERFACE:${SOLARSIM_INCLUDE_DIR}/action>
  $<BUILD_INTERFACE:${SOLARSIM_INCLUDE_DIR}/analysis>
  $<BUILD_INTERFACE:${SOLARSIM_INCLUDE_DIR}/event>
  $<BUILD_INTERFACE:${SOLARSIM_INCLUDE_DIR}/config>
  $<BUILD_INTERFACE:${SOLARSIM_INCLUDE_DIR}/geo>
  $<BUILD_INTERFACE:${SOLARSIM_INCLUDE_DIR}/gen>
  $<BUILD_INTERFACE:${SOLARSIM_INCLUDE_DIR}/physics>

  $<INSTALL_INTERFACE:${SOLARSIM_INC_DEST}>
  $<INSTALL_INTERFACE:${SOLARSIM_INC_DEST}/action>
  $<INSTALL_INTERFACE:${SOLARSIM_INC_DEST}/analysis>
  $<INSTALL_INTERFACE:${SOLARSIM_INC_DEST}/event>
  $<INSTALL_INTERFACE:${SOLARSIM_INC_DEST}/config>
  $<INSTALL_INTERFACE:${SOLARSIM_INC_DEST}/geo>
  $<INSTALL_INTERFACE:${SOLARSIM_INC_DEST}/gen>
  $<INSTALL_INTERFACE:${SOLARSIM_INC_DEST}/physics>
)
if (SLAR_CRY_INTERFACE)
  target_include_directories(solar_sim PRIVATE ${CRY_INCLUDE_DIR})
endif()
if (SLAR_RADSRC_INTERFACE)
  target_include_directories(solar_sim PRIVATE ${RADSRC_INCLUDE_DIR})
endif()
if (SLAR_USE_G4CASCADE)
  target_include_directories(solar_sim PUBLIC 
    $<BUILD_INTERFACE:${G4CASCADE_INCLUDE_DEST}>
    $<INSTALL_INTERFACE:${SOLARSIM_INC_DEST}/physics/cascade>
  )
endif()
  

# Add definitions
target_compile_definitions(solar_sim 
  PUBLIC 
  $<$<STREQUAL:${Geant4_gdml_FOUND},ON>:SLAR_GDML>
  $<$<STREQUAL:${SLAR_EXTERNAL},ON>:SLAR_EXTERNAL>
  $<$<STREQUAL:${SLAR_CRY_INTERFACE},ON>:SLAR_CRY>
  $<$<STREQUAL:${SLAR_RADSRC_INTERFACE},ON>:SLAR_RADSRC>
  SLAR_EXTERNAL_PARTICLE="${SLAR_EXTERNAL_PARTICLE}"
  PRIVATE
  "-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\""
  )

list(APPEND SOLARSIM_RPATH
  "${SOLARSIM_LIB_DIR}"
  "${BXDECAY0_DIR}/lib"
  "${BXDECAY0_DIR}/lib64"
  "${MARLEY_DIR}/build")

set_target_properties(solar_sim PROPERTIES
  INSTALL_RPATH "${SOLARSIM_RPATH}"
  BUILD_WITH_INSTALL_RPATH 1
  )

install(TARGETS solar_sim
  RUNTIME DESTINATION ${SOLARSIM_BIN_DIR}
  )

#----------------------------------------------------------------------------
# Install the headers
install(DIRECTORY ${SOLARSIM_INCLUDE_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX})

#----------------------------------------------------------------------------
# Copy all assets (geometry, materials, cfg) to the install directory.
# This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
install(DIRECTORY ${PROJECT_SOURCE_DIR}/assets DESTINATION ${CMAKE_INSTALL_PREFIX})

#----------------------------------------------------------------------------
# configure package config
include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION "${SOLARSIM_INSTALL_CMAKE_DIR}"
  PATH_VARS SOLARSIM_LIB_DIR SOLARSIM_BIN_DIR SOLARSIM_INC_DEST SOLARSIM_EXT_DIR SOLARSIM_INSTALL_CMAKE_DIR PROJECT_NAME
  )
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
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

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
  DESTINATION ${SOLARSIM_INSTALL_CMAKE_DIR}
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION ${SOLARSIM_INSTALL_CMAKE_DIR}
)


