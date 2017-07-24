project (cool.ng)
cmake_minimum_required(VERSION 3.5)

# ### ##################################################
# ###
# ### Build control variables that may get set from
# ### outside to integrate into larger projects
# ###
# ### ##################################################


# --- top of  source tree
if( NOT DEFINED COOL_NG_HOME )
  set( COOL_NG_HOME ${CMAKE_SOURCE_DIR} )
endif()

# --- build directory
if( NOT DEFINED COOL_NG_BUILD_DIR )
  set( COOL_NG_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} )
endif()

# --- enable/disable and control unit tests
if( NOT DEFINED COOL_NG_UNIT_TESTS )
  set( COOL_NG_UNIT_TESTS true)
endif()

# --- test programs location
if( COOL_NG_UNIT_TESTS )
  if( NOT DEFINED COOL_NG_TEST_DIR )
    set( COOL_NG_TEST_DIR ${COOL_NG_BUILD_DIR}/tests )
  endif()
endif()

# ### ##################################################
# ###
# ### Platform identification and compiler setup
# ###
# ### ##################################################

if( ${CMAKE_SYSTEM_NAME} MATCHES  "Darwin" )

# --- Apple OS/X
  set( BUILD_TARGET OSX_TARGET)
  set( OSX true)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC" )

elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

# --- Linux
  set( BUILD_TARGET LINUX_TARGET )
  set( LINUX true )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC" )

elseif (MSVC)

# --- Windows
  set( BUILD_TARGET WINDOWS_TARGET )
  set( WINDOWS true )

  # CMake incorrectly appends "/machine:X86" to all linker flags when static
  # libraries are used, resulting in invalid builds:
  # > fatal error LNK1112: module machine type 'x64' conflicts with target machine type 'X86'
  # (ref. https://public.kitware.com/Bug/view.php?id=15496)
  set( CMAKE_EXE_LINKER_FLAGS    "/machine:X64" )
  set( CMAKE_MODULE_LINKER_FLAGS "/machine:X64" )
  set( CMAKE_SHARED_LINKER_FLAGS "/machine:X64" )

  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /bigobj /Zm750 /MTd -D_SCL_SECURE_NO_WARNINGS")

else()

# --- unknown target
  message( FATAL "unsupported platform" )

endif()

add_definitions( "-D${BUILD_TARGET}" )
include_directories(${COOL_NG_HOME}/include)


set( COOL_NG_API_HEADERS
    include/cool/ng/async.h
    include/cool/ng/exception.h
    include/cool/ng/task.h
    include/cool/ng/traits.h
)

set( COOL_NG_IMPL_HEADERS
    include/cool/ng/impl/platform.h
    include/cool/ng/impl/task_traits.h
)

# ### ##################################################
# ###
# ### Unit tests
# ###
# ### ##################################################

set( HEADER_ONLY_UNIT_TESTS
  traits
  task-traits
)

set( traits_SRCS tests/unit/traits/traits.cpp )
set( task-traits_SRCS tests/unit/traits/task_traits.cpp)

macro(header_unit_test TestName)
  add_executable( ${TestName}-test ${ARGN} )
  if( NOT WINDOWS )
    # --- on windows we use autolink feature
    target_link_libraries( ${TestName}-test ${Boost_LIBRARIES} )
  endif()
  set_target_properties( ${TestName}-test PROPERTIES
      COMPILE_FLAGS -DBOOST_TEST_DYN_LINK
      RUNTIME_OUTPUT_DIRECTORY ${COOL_NG_TEST_DIR}
  )
  add_test( NAME ${TestName} COMMAND ${TestName}-test )
endmacro()

if( COOL_NG_UNIT_TESTS )

  # unit tests require Boost.Test library
  set( Boost_USE_STATIC_LIBS       OFF )
  set( Boost_USE_MULTITHREADED      ON )
  set( Boost_USE_STATIC_RUNTIME    OFF )

  find_package( Boost 1.58.0 COMPONENTS unit_test_framework)

  if( NOT Boost_FOUND )
    message( WARNING "Boost.Test package is required to build unit tests. Will disable unit tests compilation and proceed without Boost.Test" )
    set( COOL_NG_UNIT_TESTS false )
  endif()
endif()

if( COOL_NG_UNIT_TESTS )

  include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
  link_directories( ${Boost_LIBRARY_DIRS} )

  enable_testing()

  # --- header only unit tests
  foreach( ut ${HEADER_ONLY_UNIT_TESTS} )
    header_unit_test( ${ut} ${${ut}_SRCS} )
    set( HEADER_ONLY_UNIT_TESTS_SOURCES ${HEADER_ONLY_UNIT_TESTS_SOURCES} ${${ut}_SRCS} )
  endforeach()

endif()

# ### ##################################################
# ###
# ### Source orgenization for IDEs
# ###
# ### ##################################################

source_group( "API Header Files" FILES ${COOL_NG_API_HEADERS} )
source_group( "Impl Header Files" FILES ${COOL_NG_IMPL_HEADERS} )
source_group("Header Unit Tests" FILES ${HEADER_ONLY_UNIT_TESTS_SOURCES})

# --- custom target listing all source files (for IDEs only)
add_custom_target("All-Sources" SOURCES
  ${COOL_NG_API_HEADERS}
  ${COOL_NG_IMPL_HEADERS}
  ${HEADER_ONLY_UNIT_TESTS_SOURCES}
)

