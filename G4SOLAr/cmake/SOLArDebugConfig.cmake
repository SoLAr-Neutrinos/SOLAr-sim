#####################################################################
# @file SOLArDebugConfig.cmake
# @author Daniele Guffanti (University and INFN Milano-Bicocca)
# @date 2025-10-23 16:27
# @brief CMake script to configure debug infrastructure in SOLAr-sim
#####################################################################


# Define available debug modules with descriptions
function(define_debug_modules)
  set(DEBUG_MODULES
    "PRIMARY_EVENT:Debug primary event generation"
    "TRACKING:Debug stepping action"
    "OPTICAL:Debug optical physics"
    "DETECTOR:Debug detector construction"
    PARENT_SCOPE
  )
endfunction()

macro(configure_debug_system)
  define_debug_modules()

  option(ENABLE_DEBUG "Enable debug infrastructure" OFF)

  message(STATUS "Configuring debug infrastructure...")
  message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ENABLE_DEBUG ON CACHE BOOL "Enable debug infrastructure" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    message(STATUS "DEBUG: Adding '-g' option for gdb debugging")
  endif()

  if(NOT ENABLE_DEBUG)
    message(STATUS "Debug infrastructure disabled")
    return()
  endif()

  message(STATUS "Debug infrastructure enabled - Available modules:")
  set(SLAR_DEBUG ON)

  # Parse and create options for each module
  foreach(module_desc ${DEBUG_MODULES})
    string(REPLACE ":" ";" module_parts ${module_desc})
    list(GET module_parts 0 module_name)
    list(GET module_parts 1 module_help)

    option(DEBUG_${module_name} "${module_help}" ON)

    if(DEBUG_${module_name})
      set(DEBUG_${module_name}_AVAILABLE ON)
      add_compile_definitions(DEBUG_${module_name}_AVAILABLE)
      message(STATUS "  ✓ ${module_name}: ${module_help}")
    else()
      unset(DEBUG_${module_name}_AVAILABLE)
      message(STATUS "  ✗ ${module_name}: ${module_help}")
    endif()
  endforeach()

  # Configure header
  configure_file(
    ${PROJECT_SOURCE_DIR}/include/SLArDebugConfig.hh.in
    ${PROJECT_SOURCE_DIR}/include/SLArDebugConfig.hh
    @ONLY
  )

  add_compile_definitions(SLAR_DEBUG)
endmacro()

# Convenience function to print debug configuration
function(print_debug_config)
  if(ENABLE_DEBUG)
    message(STATUS "\n=== Debug Configuration ===")
    define_debug_modules()
    foreach(module_desc ${DEBUG_MODULES})
      string(REPLACE ":" ";" module_parts ${module_desc})
      list(GET module_parts 0 module_name)
      if(DEBUG_${module_name})
        message(STATUS "  ${module_name}: ENABLED")
      else()
        message(STATUS "  ${module_name}: DISABLED")
      endif()
    endforeach()
    message(STATUS "===========================\n")
  endif()
endfunction()

