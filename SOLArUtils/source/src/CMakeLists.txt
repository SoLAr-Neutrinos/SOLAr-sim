######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : CMakeLists
# @created     : Friday Apr 12, 2024 11:31:49 CEST
######################################################################

add_library(SLArHitConverter
  SHARED
  ${SOLAR_UTILS_SRC_DIR}/TChannelAnalyzer.cc
  ${SOLAR_UTILS_INC_DIR}/TChannelAnalyzer.hh)

target_link_libraries(SLArHitConverter PUBLIC 
  ${ROOT_LIBRARIES}
  Geant4::G4clhep Geant4::G4intercoms)

add_library(SLArEveDisplay
  SHARED
  ${SOLAR_UTILS_SRC_DIR}/SLArEveDisplay.cc
  ${SOLAR_UTILS_INC_DIR}/SLArEveDisplay.hh
  )

target_link_libraries(SLArEveDisplay PUBLIC 
  ${ROOT_LIBRARIES} ROOT::Geom ROOT::Gui ROOT::Eve ${Geant4_LIBRARIES})

ROOT_GENERATE_DICTIONARY(G__SLArEveDisplay 
  ${SOLAR_UTILS_INC_DIR}/SLArEveDisplay.hh
  MODULE SLArEveDisplay
  LINKDEF ${SOLAR_UTILS_INC_DIR}/SLArEveDisplayLinkDef.h)

#----------------------------------------------------------------------
# Install instructions

set(SOLAr_utils_libs
  SLArHitConverter
  SLArEveDisplay
  )

set(utils_resources 
  ${CMAKE_CURRENT_BINARY_DIR}/G__SLArEveDisplay.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArEveDisplay.rootmap 
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArEveDisplay_rdict.pcm
  )

foreach(lib IN LISTS SOLAr_utils_libs)
  install(TARGETS ${lib}
    #EXPORT                      G4SOLArTargets
    LIBRARY DESTINATION        "${SOLAR_UTILS_LIB_DIR}" 
    RUNTIME DESTINATION        "${SOLAR_UTILS_LIB_DIR}")
endforeach()

install(FILES ${utils_resources} DESTINATION "${SOLAR_UTILS_LIB_DIR}")

