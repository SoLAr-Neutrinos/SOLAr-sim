######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : CMakeLists.txt
# @created     : Friday Apr 12, 2024 11:31:49 CEST
######################################################################

#------------------------------------------------------------------------
# Create RecoHits library
add_library(SLArRecoHits
  SHARED
  ${SOLAR_UTILS_SRC_DIR}/SLArRecoHits.cc
  ${SOLAR_UTILS_INC_DIR}/SLArRecoHits.hh)

target_link_libraries(SLArRecoHits PRIVATE ${ROOT_LIBRARIES})

root_generate_dictionary(G__SLArRecoHits
  ${SOLAR_UTILS_INC_DIR}/SLArRecoHits.hh
  MODULE SLArRecoHits
  LINKDEF ${SOLAR_UTILS_INC_DIR}/SLArRecoHitsLinkDef.h)

#-------------------------------------------------------------------------- 
# Create HitConverter library

add_library(SLArHitConverter
  SHARED
  ${SOLAR_UTILS_SRC_DIR}/TChannelAnalyzer.cc
  ${SOLAR_UTILS_INC_DIR}/TChannelAnalyzer.hh)

target_link_libraries(SLArHitConverter PUBLIC 
  SLArRecoHits
  ${ROOT_LIBRARIES}
  Geant4::G4intercoms)

#----------------------------------------------------------------------
# Create Event Display library
set(SLAREVEDISPLAY_SOURCES
  ${SOLAR_UTILS_SRC_DIR}/SLArEveGeometry.cc
  ${SOLAR_UTILS_SRC_DIR}/SLArEveEventReader.cc
  ${SOLAR_UTILS_SRC_DIR}/SLArEveTrackRenderer.cc
  ${SOLAR_UTILS_SRC_DIR}/SLArEveHitRenderer.cc
  ${SOLAR_UTILS_SRC_DIR}/SLArEveOpHitRenderer.cc
  ${SOLAR_UTILS_SRC_DIR}/SLArEveOpHitSelectorPanel.cc
  ${SOLAR_UTILS_SRC_DIR}/SLArEveDisplay.cc
)

set(SLAREVEDISPLAY_HEADERS
  ${SOLAR_UTILS_INC_DIR}/SLArEveGeometry.hh
  ${SOLAR_UTILS_INC_DIR}/SLArEveEventReader.hh
  ${SOLAR_UTILS_INC_DIR}/SLArEveTrackRenderer.hh
  ${SOLAR_UTILS_INC_DIR}/SLArEveHitRenderer.hh
  ${SOLAR_UTILS_INC_DIR}/SLArEveOpHitSelector.hh
  ${SOLAR_UTILS_INC_DIR}/SLArEveOpHitRenderer.hh
  ${SOLAR_UTILS_INC_DIR}/SLArEveOpHitSelectorPanel.hh
  ${SOLAR_UTILS_INC_DIR}/SLArEveDisplay.hh
)     

# ── Library target ────────────────────────────────────────────────────────────
add_library(SLArEveDisplay SHARED
    ${SLAREVEDISPLAY_SOURCES}
)

target_include_directories(SLArEveDisplay
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    # SOLAr-sim headers (event/, config/, analysis/, geo/, core/).
    # Replace SOLAR_SIM_INCLUDE_DIR with the actual variable or path.
    ${SOLAR_SIM_INCLUDE_DIR}
)

target_link_libraries(SLArEveDisplay
  PUBLIC
    ROOT::Core
    ROOT::Hist
    ROOT::Tree
    ROOT::RIO
    ROOT::Eve
    ROOT::Gui
    ROOT::GuiHtml
    ROOT::XMLIO
    ROOT::GeomPainter
    ROOT::Geom
    ROOT::Physics
    ROOT::Graf
    ROOT::Graf3d
    ${Geant4_LIBRARIES}
  PRIVATE
    # Link against the SOLAr-sim runtime library for event / config objects.
    # Replace with your actual target name (e.g. SOLArSimLib).
    SLArRecoHits
    SOLARSIM::SLArReadoutSystemConfig
    SOLARSIM::SLArMCTruth
    SOLARSIM::SLArMCEventReadout
  )

root_generate_dictionary(G_SLArEveDisplay
  ${SOLAR_UTILS_INC_DIR}/SLArEveDisplay.hh
  ${SOLAR_UTILS_INC_DIR}/SLArEveTrackRenderer.hh 
  MODULE SLArEveDisplay
  LINKDEF ${SOLAR_UTILS_INC_DIR}/SLArEveDisplayLinkDef.h
  OPTIONS -I${SOLAR_UTILS_INC_DIR}
)


#----------------------------------------------------------------------
# Install instructions
set(SOLAr_utils_libs
  SLArRecoHits
  SLArHitConverter
  SLArEveDisplay
  )

set(utils_resources)

foreach(lib IN LISTS SOLAr_utils_libs)
  # check if dictionary file exists
  if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/G__${lib}.cxx")
    message(STATUS "Dictionary file for ${lib} found")
    # add dictionary files to the list of resources
    list(APPEND utils_resources 
      "${CMAKE_CURRENT_BINARY_DIR}/G__${lib}.cxx"
      "${CMAKE_CURRENT_BINARY_DIR}/lib${lib}.rootmap"
      "${CMAKE_CURRENT_BINARY_DIR}/lib${lib}_rdict.pcm"
  )
  endif()

  # set the library properties
  set_target_properties(${lib} PROPERTIES
    BUILD_RPATH ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_RPATH ${SOLAR_UTILS_LIB_DIR}
  )

  # install the library
  install(TARGETS ${lib}
    LIBRARY DESTINATION        "${SOLAR_UTILS_LIB_DIR}" 
    RUNTIME DESTINATION        "${SOLAR_UTILS_LIB_DIR}")
endforeach()

# install the dictionary files
install(FILES ${utils_resources} DESTINATION "${SOLAR_UTILS_LIB_DIR}")

# install the headers
install(DIRECTORY ${SOLAR_UTILS_INC_DIR}/ DESTINATION "${SOLAR_UTILS_INC_DEST}" FILES_MATCHING PATTERN "*.hh")

