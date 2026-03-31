set(SLAR_GEN_INCLUDE_DIR "${SOLARSIM_INCLUDE_DIR}/gen")
set(SLAR_GEN_SOURCE_DIR "${SOLARSIM_SRC_DIR}/gen")

set(SLAR_GEN_HEADERS
  "${SLAR_GEN_INCLUDE_DIR}/SLArRandomExtra.hh"
  # vertex generators
  "${SLAR_GEN_INCLUDE_DIR}/SLArVertexGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArPointVertexGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArGPSVertexGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArBulkVertexGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArBoxSurfaceVertexGenerator.hh"
  # direction generators
  "${SLAR_GEN_INCLUDE_DIR}/SLArDirectionGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArFixedDirectionGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArGPSDirectionGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArIsotropicDirectionGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArSunDirectionGenerator.hh"
  # primary generators
  "${SLAR_GEN_INCLUDE_DIR}/SLArBaseGenerator.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArPGunGeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArPBombGeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArDecay0GeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArDecay0GeneratorMessenger.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArMarleyGeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArExternalGeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArGENIEGeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArPrimaryGeneratorAction.hh"
  "${SLAR_GEN_INCLUDE_DIR}/SLArPrimaryGeneratorMessenger.hh"
)

set(SLAR_GEN_SRC
  "${SLAR_GEN_SOURCE_DIR}/SLArRandomExtra.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArPointVertexGenerator.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArBulkVertexGenerator.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArGPSVertexGenerator.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArBoxSurfaceVertexGenerator.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArGPSDirectionGenerator.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArSunDirectionGenerator.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArBaseGenerator.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArPGunGeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArPBombGeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArDecay0GeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArDecay0GeneratorMessenger.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArMarleyGeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArExternalGeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArGENIEGeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArPrimaryGeneratorAction.cc"
  "${SLAR_GEN_SOURCE_DIR}/SLArPrimaryGeneratorMessenger.cc"
)

if (SLAR_CRY_INTERFACE)
  list(APPEND SLAR_GEN_HEADERS
    "${SLAR_GEN_INCLUDE_DIR}/SLArCRYGeneratorAction.hh"
  )
  list(APPEND SLAR_GEN_SRC
    "${SLAR_GEN_SOURCE_DIR}/SLArCRYGeneratorAction.cc"
  )
endif()

if (SLAR_RADSRC_INTERFACE)
  list(APPEND SLAR_GEN_HEADERS
    "${SLAR_GEN_INCLUDE_DIR}/SLArRadSrcGeneratorAction.hh"
  )
  list(APPEND SLAR_GEN_SRC
    "${SLAR_GEN_SOURCE_DIR}/SLArRadSrcGeneratorAction.cc"
  )
endif()

set(solarsim_headers ${solarsim_headers} ${SLAR_GEN_HEADERS} PARENT_SCOPE)
set(solarsim_sources ${solarsim_sources} ${SLAR_GEN_SRC} PARENT_SCOPE)
