########################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : CMakeLists.txt
# @created     : Tuesday Oct 18, 2022 11:31:06 CEST
# @description : Analysis libraries to implement a visibility map 
#                based on the semi-analytical model of light propagation
########################################################################

add_library(SLArLightPropagation SHARED
  ${SOLARSIM_ANALYSIS_SRC_DIR}/SLArLightPropagationModel.cpp
  ${SOLARSIM_ANALYSIS_INC_DIR}/SLArLightPropagationModel.hh
  ${SOLARSIM_ANALYSIS_INC_DIR}/SLArLightPropagationPars.hpp
  )

target_link_libraries(SLArLightPropagation PUBLIC ${ROOT_LIBRARIES})
target_link_libraries(SLArLightPropagation PUBLIC SOLARSIM::SLArReadoutSystemConfig)

ROOT_GENERATE_DICTIONARY(G__SLArLightPropagation
  ${SOLARSIM_ANALYSIS_INC_DIR}/SLArLightPropagationModel.hh
  MODULE SLArLightPropagation
  LINKDEF ${SOLARSIM_ANALYSIS_INC_DIR}/SLArLightPropagationLinkDef.h)

#----------------------------------------------------------------------
# Install instructions

set(solarsim_analysis_libs
  SLArLightPropagation
  )

set(solarsim_analysis_resources 
  ${CMAKE_CURRENT_BINARY_DIR}/G__SLArLightPropagation.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArLightPropagation.rootmap 
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArLightPropagation_rdict.pcm
  )

foreach(lib IN LISTS solarsim_analysis_libs)
  install(TARGETS ${lib}
    EXPORT                      SOLArSimAnalysisTargets
    LIBRARY DESTINATION        "${SOLARSIM_ANALYSIS_LIB_DIR}" 
    RUNTIME DESTINATION        "${SOLARSIM_ANALYSIS_LIB_DIR}")
endforeach()

install(FILES ${solarsim_analysis_resources} DESTINATION "${SOLARSIM_ANALYSIS_LIB_DIR}")

