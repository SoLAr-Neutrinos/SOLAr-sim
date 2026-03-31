######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : CMakeLists.txt
# @created     : Monday Apr 08, 2024 14:11:35 CEST
######################################################################

list(APPEND CMAKE_INSTALL_RPATH "${SOLARSIM_LIB_DIR}" "${SOLAR_UTILS_LIB_DIR}" )
message(STATUS "CMAKE_INSTALL_RPATH: ${CMAKE_INSTALL_RPATH}")

add_executable(hit_converter hit_converter.cc)
target_link_libraries(hit_converter PUBLIC
  SOLARSIM::SLArMCEventReadout
  SOLARSIM::SLArReadoutSystemConfig 
  SLArRecoHits
  SLArHitConverter
  )
target_include_directories(hit_converter
  PRIVATE
  $<BUILD_INTERFACE:${SOLAR_UTILS_INC_DIR}>
  $<INSTALL_INTERFACE:${SOLAR_UTILS_INC_DEST}>
  ${SOLARSIM_INCLUDE_DIR}
  ${Geant4_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIRS}
  "${RapidJSON_INCLUDE_DIR}/.."
  )

add_executable(hit_viewer hit_viewer.cc)
target_link_libraries(hit_viewer 
  PUBLIC SLArEveDisplay
  PRIVATE
  ${ROOT_LIBRARIES} ROOT::Geom
  SOLARSIM::SLArMCTruth
  SOLARSIM::SLArMCEventReadout
  SOLARSIM::SLArReadoutSystemConfig 
  )
target_include_directories(hit_viewer
  PRIVATE
  $<BUILD_INTERFACE:${SOLAR_UTILS_INC_DIR}>
  $<INSTALL_INTERFACE:${SOLAR_UTILS_INC_DEST}>
  ${SOLARSIM_INCLUDE_DIR}
  ${Geant4_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIRS}
  "${RapidJSON_INCLUDE_DIR}/.."
  )

add_executable(hit_calibration hit_calibration.cc)
target_link_libraries(hit_calibration
  PRIVATE
  ${ROOT_LIBRARIES} 
  SOLARSIM::SLArGenRecords
)
target_include_directories(hit_calibration
  PRIVATE
  ${SOLARSIM_INCLUDE_DIR}
  ${Geant4_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIRS}
  "${RapidJSON_INCLUDE_DIR}/.."
  )

add_executable(hit_cavern hit_cavern.cc)
target_link_libraries(hit_cavern
  PRIVATE
  ${ROOT_LIBRARIES} 
  SOLARSIM::SLArGenRecords
)
target_include_directories(hit_cavern
  PRIVATE
  ${SOLARSIM_INCLUDE_DIR}
  ${Geant4_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIRS}
  "${RapidJSON_INCLUDE_DIR}"
  )

add_executable(etrue_cavern etrue_cavern.cc)
target_link_libraries(etrue_cavern
  PRIVATE 
  ${ROOT_LIBRARIES}
  SOLARSIM::SLArGenRecords
  SOLARSIM::SLArMCTruth
  SOLARSIM::SLArReadoutSystemConfig
)
target_include_directories(etrue_cavern
  PRIVATE
  ${SOLARSIM_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIRS}
)

install(TARGETS hit_converter hit_viewer hit_calibration hit_cavern etrue_cavern
  LIBRARY DESTINATION "${SOLAR_UTILS_BIN_DIR}"
  RUNTIME DESTINATION "${SOLAR_UTILS_BIN_DIR}"
  )

