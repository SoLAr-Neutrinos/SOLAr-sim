set(SLAR_GEO_DETECTOR_HEADERS 
  "${SLAR_GEO_INCLUDE_DIR}/detector/SLArDetectorConstruction.hh"
  "${SLAR_GEO_INCLUDE_DIR}/detector/SLArDetectorConstructionMsgr.hh"
  "${SLAR_GEO_INCLUDE_DIR}/detector/SLArBaseDetModule.hh"
  "${SLAR_GEO_INCLUDE_DIR}/detector/SLArPlaneParameterisation.hpp"
)

set(SLAR_GEO_DETECTOR_SRC 
  "${SLAR_GEO_SRC_DIR}/detector/SLArDetectorConstruction.cc"
  "${SLAR_GEO_SRC_DIR}/detector/SLArDetectorConstructionMsgr.cc"
  "${SLAR_GEO_SRC_DIR}/detector/SLArBaseDetModule.cc"
)

add_subdirectory( Hall )
add_subdirectory( TPC )
add_subdirectory( Anode )
add_subdirectory( SuperCell )

set(SLAR_GEO_HEADERS ${SLAR_GEO_HEADERS} ${SLAR_GEO_DETECTOR_HEADERS} PARENT_SCOPE)
set(SLAR_GEO_SRC ${SLAR_GEO_SRC} ${SLAR_GEO_DETECTOR_SRC} PARENT_SCOPE)
