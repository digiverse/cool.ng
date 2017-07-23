project (cool.ng)
cmake_minimum_required(VERSION 3.5)

# control variables
if( NOT DEFINED COOL_NG_HOME )
  set( COOL_NG_HOME ${CMAKE_SOURCE_DIR} )
endif()

if( NOT DEFINED COOL_NG_BUILD_DIR )
  set( COOL_NG_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} )
endif()

# -- enable/disable and control unit tests
if( NOT DEFINED COOL_NG_UNIT_TESTS )
  set( COOL_NG_UNIT_TESTS true)
endif()

if( COOL_NG_UNIT_TESTS )
  if( NOT DEFINED COOL_NG_TEST_DIR )
    set( COOL_NG_TEST_DIR ${COOL_NG_BUILD_DIR}/tests )
  endif()
endif()

include_directories(${COOL_NG_HOME}/include)

if (WINDOWS)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /EHsc /MTd")
else()
  set (CMAKE_CXX_FLAGS "-std=c++11 -g ${DISABLED_WARNINGS} -fpermissive")
endif()

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


add_custom_target(helper SOURCES ${COOL_NG_API_HEADERS} ${COOL_NG_IMPL_HEADERS})

# ### Unit tests

macro(unit_test TestName)
  add_executable( ${TestName}-test ${ARGN} )
  target_link_libraries( ${TestName}-test ${Boost_LIBRARIES} )
  set_target_properties( ${TestName}-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${COOL_NG_TEST_DIR} )
  add_test( ${TestName} COMMAND ${COOL_NG_TEST_DIR}/${TestName}-test )
endmacro()

if( COOL_NG_UNIT_TESTS )

  # unit tests require Boost.Test library
  set( Boost_NO_BOOST_CMAKE         ON )
  set( Boost_USE_STATIC_LIBS        ON )
  set( Boost_USE_MULTITHREADED     OFF )
  set( Boost_USE_STATIC_RUNTIME     ON )

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

  unit_test( traits tests/unit/traits/traits.cpp )
  unit_test( task_traits tests/unit/traits/task_traits.cpp )

endif()
