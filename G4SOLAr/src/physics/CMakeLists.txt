set(SLAR_PHYSICS_INCLUDE_DIR ${SOLARSIM_INCLUDE_DIR}/physics)
set(SLAR_PHYSICS_SOURCE_DIR ${SOLARSIM_SRC_DIR}/physics)

set(SLAR_PHYSICS_HEADERS 
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArStepMax.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArExtraPhysics.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArScintillation.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArOpWLS.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArOpticalPhysics.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArFastLightSim.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArFLSPhotonLibrary.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArFastLightSimMessenger.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArPhysicsList.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArPhysicsListMessenger.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArCrossSectionBiasing.hh"
  "${SLAR_PHYSICS_INCLUDE_DIR}/SLArElectronDrift.hh"

)

set(SLAR_PHYSICS_SOURCES
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArStepMax.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArExtraPhysics.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArScintillation.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArOpWLS.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArOpticalPhysics.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArFastLightSim.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArFLSPhotonLibrary.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArFastLightSimMessenger.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArPhysicsList.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArPhysicsListMessenger.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArCrossSectionBiasing.cc"
  "${SLAR_PHYSICS_SOURCE_DIR}/SLArElectronDrift.cc"
)

add_subdirectory( LiquidArgon )

if (SLAR_USE_G4CASCADE) 
  add_subdirectory( cascade )
endif()

set(solarsim_headers ${solarsim_headers} ${SLAR_PHYSICS_HEADERS} PARENT_SCOPE)
set(solarsim_sources ${solarsim_sources} ${SLAR_PHYSICS_SOURCES} PARENT_SCOPE)
