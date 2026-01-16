########################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : CMakeLists.txt
# @created     : Tuesday Oct 18, 2022 11:31:06 CEST
# @description : Analysis libraries to implement a visibility map 
#                based on the semi-analytical model of light propagation
########################################################################

cmake_minimum_required(VERSION 3.4)
project(SLArAnalysis)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ROOT REQUIRED COMPONENTS RIO Core MathMore)
include(${ROOT_USE_FILE})


find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})

find_package(SOLAr-sim)
if(SOLAr-sim_FOUND)
  message(STATUS "SOLAr-sim found: header files at ${SOLARSIM_INCLUDE_DIR}")
  list(APPEND CMAKE_PREFIX_PATH ${SOLARSIM_BIN_DIR})
endif()

#----------------------------------------------------------------------------
# Define path variables

list(APPEND CMAKE_LIBRARY_PATH ${SOLARSIM_LIB_DIR})
set(SOLARSIM_ANALYSIS_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib")
set(SOLARSIM_ANALYSIS_BIN_DIR "${CMAKE_INSTALL_PREFIX}")
set(SOLARSIM_ANALYSIS_INC_DIR "${PROJECT_SOURCE_DIR}/source/include")
set(SOLARSIM_ANALYSIS_SRC_DIR "${PROJECT_SOURCE_DIR}/source/src")
set(SOLARSIM_ANALYSIS_SCRIPT_DIR "${PROJECT_SOURCE_DIR}/source/script")
set(SOLARSIM_INSTALL_CMAKE_DIR "${SOLARSIM_ANALYSIS_LIB_DIR}/cmake/${PROJECT_NAME}")

include(ExternalProject)
ExternalProject_Add("progressbar"
  SOURCE_DIR      ${CMAKE_CURRENT_BINARY_DIR}/progressbar
  INSTALL_DIR     ${G4S_ANALYSIS_INC_DIR}
  UPDATE_DISCONNECTED 1
  GIT_REPOSITORY  "https://github.com/dguff/progressbar.git"
  GIT_TAG         "main"
  CONFIGURE_COMMAND echo "configuring progressbar"
  BUILD_COMMAND echo "fetching progressbar"
  INSTALL_COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/progressbar/include/progressbar.hpp ${SOLARSIM_ANALYSIS_INC_DIR} )


include_directories(
  ${SOLARSIM_ANALYSIS_INC_DIR}
  ${SOLARSIM_INCLUDE_DIR}
  ${Geant4_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIRS}
)
if (CLHEP_EXTERNAL) 
  include_directories(${CLHEP_EXTERNAL})
endif()

link_directories(${ROOT_LIBRARY_DIR})
#----------------------------------------------------------------------------
# Build Analysis libraries
add_subdirectory(${SOLARSIM_ANALYSIS_SRC_DIR})

#----------------------------------------------------------------------------
# configure package config
include(CMakePackageConfigHelpers)

# configure package config
#export(EXPORT G4SAnalysisTargets
  #FILE "${G4S_INSTALL_CMAKE_DIR}/G4SAnalysisTargets.cmake"
  #NAMESPACE G4SOLAr::
#)

# configure rootlogon and setup script
configure_file(${PROJECT_SOURCE_DIR}/source/rootlogon.C.in 
               ${PROJECT_SOURCE_DIR}/rootlogon.C @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/source/SLArAnalysisLibSetup.hpp.in 
               ${SOLARSIM_ANALYSIS_INC_DIR}/SLArAnalysisLibSetup.hpp @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/source/setup_g4s_analysis.sh.in 
               ${SOLARSIM_ANALYSIS_BIN_DIR}/setup_g4s_analysis.sh @ONLY)
#----------------------------------------------------------------------------
# Build analysis executables
add_subdirectory(${SOLARSIM_ANALYSIS_SCRIPT_DIR})

