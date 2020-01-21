#
# Inputs:
#   COOL_NG_HOME          top of source tree
#   COOL_NG_BUILD_DIR     build directory
#   COOL_NG_UNIT_TESTS    build unit tests (true/false)
#   COOL_NG_BUILD_DOC     build documentation (true/false)
#   COOL_NG_LIB_DIR       output directory for libraries
#   COOL_NG_BIN_DIR       output directory for binaries
#   COOL_NG_DOC_DIR       output directory for documentation
#   COOL_NG_DEV_LIBS      build development libraries (true/false)
#
# Outputs:
#   COOL_NG_COMPONENT_ARCHIVES              list of archive libraries provided by the component
#   COOL_NG_COMPONENT_LIBRARIES             list of shared (dynamic) libraries provided by the component
#   COOL_NG_COMPONENT_DEPENDS               list of libraries (static or dynamic) this component depends on
#   COOL_NG_COMPONENT_INCLUDE_DIRECTORIES   list of include directories for component
#


# --- whether to compile development library versions or not
if( NOT DEFINED COOL_NG_DEV_LIBS )
  set ( COOL_NG_DEV_LIBS true)
endif()


# ### ##################################################
# ###
# ### Cool library
# ###
# ### ##################################################

include_directories( ${COOL_NG_COMPONENT_INCLUDE_DIRECTORIES} )

set( COOL_NG_COMPONENT_ARCHIVES  ${COOL_NG_LIB_STATIC} )
set( COOL_NG_COMPONENT_LIBRARIES ${COOL_NG_LIB_DYNAMIC} )
set( COOL_NG_COMPONENT_DEPENDS   ${COOL_NG_PLATFORM_LIBRARIES} )


# --- development libraries
add_library( ${COOL_NG_TARGET_STATIC}  STATIC ${COOL_NG_BUILD_FILES} )
add_library( ${COOL_NG_TARGET_DYNAMIC} SHARED ${COOL_NG_BUILD_FILES} )

target_include_directories( ${COOL_NG_TARGET_STATIC}  BEFORE PRIVATE ${COOL_NG_HOME}/lib ${COOL_NG_HOME}/lib/include )
target_include_directories( ${COOL_NG_TARGET_STATIC}  PUBLIC  ${COOL_NG_COMPONENT_INCLUDE_DIRECTORIES} )
target_include_directories( ${COOL_NG_TARGET_DYNAMIC} BEFORE PRIVATE ${COOL_NG_HOME}/lib ${COOL_NG_HOME}/lib/include )
target_include_directories( ${COOL_NG_TARGET_DYNAMIC} PUBLIC  ${COOL_NG_COMPONENT_INCLUDE_DIRECTORIES} )
target_compile_definitions( ${COOL_NG_TARGET_STATIC}  PRIVATE COOL_NG_BUILD COOL_NG_STATIC_LIBRARY ${COOL_ASYNC_PLATFORM} ${TASK_RUNNER_IMPL} )
target_compile_definitions( ${COOL_NG_TARGET_DYNAMIC} PRIVATE COOL_NG_BUILD ${COOL_ASYNC_PLATFORM} ${TASK_RUNNER_IMPL} )
target_compile_definitions( ${COOL_NG_TARGET_DYNAMIC} PUBLIC  PLATFORM_TARGET=${COOL_PLATFORM_TARGET} )
target_compile_definitions( ${COOL_NG_TARGET_STATIC}  PUBLIC  PLATFORM_TARGET=${COOL_PLATFORM_TARGET} )

# --- now set file names and locations
if( NOT WINDOWS )
  set_target_properties( ${COOL_NG_TARGET_DYNAMIC} PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "${COOL_NG_LIB_DYNAMIC}"
    LIBRARY_OUTPUT_DIRECTORY "${COOL_NG_LIB_DIR}"
  )
  set_target_properties( ${COOL_NG_TARGET_STATIC}  PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "${COOL_NG_LIB_STATIC}"
    ARCHIVE_OUTPUT_DIRECTORY "${COOL_NG_LIB_DIR}"
  )

  target_compile_options( ${COOL_NG_TARGET_DYNAMIC} PUBLIC -g -std=c++11 -fPIC )
  target_compile_options( ${COOL_NG_TARGET_STATIC}  PUBLIC -g -std=c++11 -fPIC )

  if (OSX)
    
    set_target_properties( ${COOL_NG_TARGET_DYNAMIC} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
    target_compile_options( ${COOL_NG_TARGET_DYNAMIC} PUBLIC -fbracket-depth=10000 )
    target_compile_definitions( ${COOL_NG_TARGET_DYNAMIC} PUBLIC OS_OBJECT_USE_OBJC=0 )
    target_compile_options( ${COOL_NG_TARGET_STATIC} PUBLIC -fbracket-depth=10000 )
    target_compile_definitions( ${COOL_NG_TARGET_STATIC} PUBLIC OS_OBJECT_USE_OBJC=0 )

  endif()

else()

  set_target_properties( ${COOL_NG_TARGET_DYNAMIC} PROPERTIES
    PREFIX ""
    OUTPUT_NAME "${COOL_NG_LIB_DYNAMIC}"
    RUNTIME_OUTPUT_DIRECTORY "${COOL_NG_BIN_DIR}"
  )
  target_link_libraries( ${COOL_NG_TARGET_DYNAMIC} ${COOL_NG_PLATFORM_LIBRARIES} )
  set_target_properties( ${COOL_NG_TARGET_STATIC} PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "${COOL_NG_LIB_STATIC}"
    ARCHIVE_OUTPUT_DIRECTORY "${COOL_NG_LIB_DIR}"
  )
  target_link_libraries( ${COOL_NG_TARGET_STATIC} ${COOL_NG_PLATFORM_LIBRARIES} )

  target_compile_options(     ${COOL_NG_TARGET_DYNAMIC} PUBLIC /EHsc /bigobj /Zm750 )
  target_compile_options(     ${COOL_NG_TARGET_STATIC}  PUBLIC /EHsc /bigobj /Zm750 )
  target_compile_definitions( ${COOL_NG_TARGET_DYNAMIC} PUBLIC _SCL_SECURE_NO_WARNINGS )
  target_compile_definitions( ${COOL_NG_TARGET_STATIC}  PUBLIC _SCL_SECURE_NO_WARNINGS )

endif()
