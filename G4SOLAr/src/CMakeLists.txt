######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : CMakeLists.txt
# @created     : Thursday Oct 23, 2025 17:26:30 CEST
######################################################################

set( SLAR_ROOT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include" )
set( SLAR_ROOT_SRC_DIR "${PROJECT_SOURCE_DIR}/src" )

set ( SLAR_ROOT_HEADERS
  "${SLAR_ROOT_INCLUDE_DIR}/SLArRootUtilities.hh"
  "${SLAR_ROOT_INCLUDE_DIR}/SLArDebugManager.hh"
  "${SLAR_ROOT_INCLUDE_DIR}/SLArDebugConfig.hh"
)

set( SLAR_ROOT_SRC 
  "${SLAR_ROOT_SRC_DIR}/SLArDebugManager.cc"
)

set( solarsim_headers ${solarsim_headers} ${SLAR_ROOT_HEADERS} PARENT_SCOPE )
set( solarsim_sources ${solarsim_sources} ${SLAR_ROOT_SRC} PARENT_SCOPE )

