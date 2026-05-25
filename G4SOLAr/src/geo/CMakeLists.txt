######################################################################
# @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
# @file        : CMakeLists.txt
# @created     : Tuesday Nov 26, 2024 16:14:13 CET
######################################################################

set( SLAR_GEO_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/geo" )
set( SLAR_GEO_SRC_DIR "${PROJECT_SOURCE_DIR}/src/geo" )

set ( SLAR_GEO_HEADERS
  "${SLAR_GEO_INCLUDE_DIR}/SLArUnit.hpp"
  "${SLAR_GEO_INCLUDE_DIR}/SLArGeoInfo.hh"
  "${SLAR_GEO_INCLUDE_DIR}/SLArGeoUtils.hh"
)

set( SLAR_GEO_SRC 
  "${SLAR_GEO_SRC_DIR}/SLArGeoInfo.cc"
  "${SLAR_GEO_SRC_DIR}/SLArGeoUtils.cc"
)

add_subdirectory( material )
add_subdirectory( detector )

set( solarsim_headers ${solarsim_headers} ${SLAR_GEO_HEADERS} PARENT_SCOPE )
set( solarsim_sources ${solarsim_sources} ${SLAR_GEO_SRC} PARENT_SCOPE )
