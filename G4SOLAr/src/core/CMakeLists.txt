set(SLAR_CORE_INCLUDE_DIR "${SOLARSIM_INCLUDE_DIR}/core")
set(SLAR_CORE_SOURCE_DIR "${SOLARSIM_SRC_DIR}/core")

set(SLAR_CORE_HEADERS
  "${SLAR_CORE_INCLUDE_DIR}/SLArDebugConfig.hh"
  "${SLAR_CORE_INCLUDE_DIR}/SLArDebugUtils.hh"
  "${SLAR_CORE_INCLUDE_DIR}/SLArRootUtilities.hh"
)

set(SLAR_CORE_SRC
  "${SLAR_CORE_SOURCE_DIR}/SLArRootUtilities.cc"
)

set(solarsim_headers ${solarsim_headers} ${SLAR_CORE_HEADERS} PARENT_SCOPE)
set(solarsim_sources ${solarsim_sources} ${SLAR_CORE_SRC} PARENT_SCOPE)
