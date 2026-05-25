######################################################################
# @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
# @file        : CMakeLists
# @created     : Wed Jun 22, 2022 15:14:17 CEST
######################################################################

set(SLAR_CFG_INCLUDE_DIR ${SOLARSIM_INCLUDE_DIR}/config)
set(SLAR_CFG_SRC_DIR ${SOLARSIM_SRC_DIR}/config)

set(SLAR_CFG_HEADERS 
  ${SLAR_CFG_INCLUDE_DIR}/SLArCfgBaseModule.hh
  ${SLAR_CFG_INCLUDE_DIR}/SLArCfgSuperCell.hh
  ${SLAR_CFG_INCLUDE_DIR}/SLArCfgReadoutTile.hh
  ${SLAR_CFG_INCLUDE_DIR}/SLArCfgAssembly.hh
  ${SLAR_CFG_INCLUDE_DIR}/SLArCfgSuperCellArray.hh
  ${SLAR_CFG_INCLUDE_DIR}/SLArCfgMegaTile.hh
  ${SLAR_CFG_INCLUDE_DIR}/SLArCfgBaseSystem.hh
  ${SLAR_CFG_INCLUDE_DIR}/SLArCfgAnode.hh
  )
set(SLAR_CFG_SOURCES 
  ${SLAR_CFG_SRC_DIR}/SLArCfgBaseModule.cc
  ${SLAR_CFG_SRC_DIR}/SLArCfgSuperCell.cc
  ${SLAR_CFG_SRC_DIR}/SLArCfgReadoutTile.cc
  ${SLAR_CFG_SRC_DIR}/SLArCfgAssembly.cc
  ${SLAR_CFG_SRC_DIR}/SLArCfgSuperCellArray.cc
  ${SLAR_CFG_SRC_DIR}/SLArCfgMegaTile.cc
  ${SLAR_CFG_SRC_DIR}/SLArCfgBaseSystem.cc
  ${SLAR_CFG_SRC_DIR}/SLArCfgAnode.cc
  )

# SLArReadoutSystemConfig dictionary 
add_library(SLArReadoutSystemConfig SHARED
  ${SLAR_CFG_SOURCES} ${SLAR_CFG_HEADERS}
)
target_link_libraries(SLArReadoutSystemConfig PRIVATE ${ROOT_LIBRARIES})
target_include_directories(SLArReadoutSystemConfig
  PUBLIC
  $<BUILD_INTERFACE:${SOLARSIM_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:${SOLARSIM_INC_DEST}>
)
ROOT_GENERATE_DICTIONARY(G__SLArReadoutSystemConfig 
  ${SOLARSIM_INCLUDE_DIR}/config/*.hh
  MODULE SLArReadoutSystemConfig
  LINKDEF ${SOLARSIM_INCLUDE_DIR}/config/SLArReadoutSystemConfigLinkDef.h)

#----------------------------------------------------------------------
# Include files in header and sources list
set(solarsim_headers ${solarsim_headers} ${SLAR_CFG_HEADERS} PARENT_SCOPE)
set(solarsim_sources ${solarsim_sources} ${SLAR_CFG_SOURCES} PARENT_SCOPE)

#----------------------------------------------------------------------
# Install instructions

set(G4SOLAr_cfg_libs
  SLArReadoutSystemConfig
  )

set(G4SOLAr_cfg_resources 
  ${CMAKE_CURRENT_BINARY_DIR}/G__SLArReadoutSystemConfig.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArReadoutSystemConfig.rootmap 
  ${CMAKE_CURRENT_BINARY_DIR}/libSLArReadoutSystemConfig_rdict.pcm
  )

foreach(lib IN LISTS G4SOLAr_cfg_libs)
  set_target_properties(${lib} PROPERTIES 
    BUILD_RPATH ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_RPATH ${SOLARSIM_LIB_DIR}
  )

  install(TARGETS ${lib}
    EXPORT                     "${PROJECT_NAME}Targets" 
    LIBRARY DESTINATION        "${SOLARSIM_LIB_DIR}" 
    RUNTIME DESTINATION        "${SOLARSIM_LIB_DIR}")
endforeach()

install(FILES ${G4SOLAr_cfg_resources} DESTINATION "${SOLARSIM_LIB_DIR}")
