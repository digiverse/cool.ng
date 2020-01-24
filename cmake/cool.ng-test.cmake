
set( HEADER_ONLY_UNIT_TESTS
  traits
  task-traits
)

# unit tests for library internals, require static lib owing to MS dll export/import rules
set( LIBRARY_UNIT_TESTS
#  executor
  run_queue
)

# api level unit tests, will use both static and dynamic library
set( API_UNIT_TESTS
  utilities
  ip_address
  error
#  task
#  es_reader
#  es_timer
)

### Test files

# Header only tests
set( traits_SRCS            tests/unit/traits/traits.cpp )
set( task-traits_SRCS       tests/unit/traits/task_traits.cpp )

# Internal tests
set( executor_SRCS          tests/unit/executor/executor.cpp )
set( run_queue_SRCS         tests/unit/run_queue/run_queue.cpp )

# API level tests
set( utilities_SRCS tests/unit/utilities/binary.cpp  tests/unit/utilities/identification.cpp)
set( ip_address_SRCS tests/unit/net/ip_address.cpp )
set( error_SRCS tests/unit/error/error.cpp)
set( task_SRCS tests/unit/task/task_common.h tests/unit/task/task_common.cpp
  tests/unit/task/simple_task.cpp
  tests/unit/task/intercept_task.cpp
  tests/unit/task/sequential_task.cpp
  tests/unit/task/conditional_task.cpp
  tests/unit/task/repeat_task.cpp
  tests/unit/task/loop_task.cpp
)
set( es_reader_SRCS tests/unit/event_sources/es_reader.cpp )
set( es_timer_SRCS tests/unit/event_sources/es_timer.cpp )

### Helper macros

macro(header_unit_test TestName)
  add_executable( ${TestName}-test ${ARGN} )

#  target_link_directories( ${TestName}-test PRIVATE ${Boost_LIBRARY_DIRS} )
  target_link_libraries( ${TestName}-test ${Boost_LIBRARIES} ${COOL_NG_PLATFORM_LIBRARIES} )
  target_include_directories( ${TestName}-test SYSTEM PRIVATE ${Boost_INCLUDE_DIR} )
  target_include_directories( ${TestName}-test PRIVATE ${COOL_NG_HOME}/tests/unit ${COOL_NG_COMPONENT_INCLUDE_DIRECTORIES})
  target_compile_definitions( ${TestName}-test PRIVATE BOOST_TEST_DYN_LINK )
  target_compile_options( ${TestName}-test PRIVATE ${COOL_NG_COMPILER_OPTIONS} )
  set_target_properties( ${TestName}-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${COOL_NG_TEST_DIR}
    LINK_DIRECTORIES  ${Boost_LIBRARY_DIRS}
    FOLDER "Unit Tests/Header Only"
  )
  add_test( NAME ${TestName} COMMAND ${TestName}-test )
endmacro()

macro(internal_unit_test TestName)
  add_executable( ${TestName}-test ${ARGN} )

#  target_link_directories( ${TestName}-test PRIVATE ${Boost_LIBRARY_DIRS} )
  target_link_libraries( ${TestName}-test cool.ng.archive ${Boost_LIBRARIES} ${COOL_NG_PLATFORM_LIBRARIES} )
  target_include_directories( ${TestName}-test SYSTEM PRIVATE ${Boost_INCLUDE_DIR} )
  target_include_directories( ${TestName}-test PRIVATE ${COOL_NG_HOME}/tests/unit )
  target_compile_definitions( ${TestName}-test PRIVATE BOOST_TEST_DYN_LINK COOL_NG_STATIC_LIBRARY )
   set_target_properties( ${TestName}-test PROPERTIES
     RUNTIME_OUTPUT_DIRECTORY ${COOL_NG_TEST_DIR}
     LINK_DIRECTORIES  ${Boost_LIBRARY_DIRS}
     FOLDER "Unit Tests/Internal"
  )
  add_test( NAME ${TestName} COMMAND ${TestName}-test )
endmacro()

macro(api_unit_test TestName)
  add_executable( ${TestName}-test ${ARGN} )
  add_executable( ${TestName}-test-dyn ${ARGN} )

#  target_link_directories( ${TestName}-test PRIVATE ${Boost_LIBRARY_DIRS} )
  target_link_libraries( ${TestName}-test cool.ng.archive ${Boost_LIBRARIES} )
  target_include_directories( ${TestName}-test SYSTEM PRIVATE ${Boost_INCLUDE_DIR} )
  target_include_directories( ${TestName}-test PRIVATE ${COOL_NG_HOME}/tests/unit )
  target_compile_definitions( ${TestName}-test PRIVATE BOOST_TEST_DYN_LINK )
  set_target_properties( ${TestName}-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${COOL_NG_TEST_DIR}
    LINK_DIRECTORIES  ${Boost_LIBRARY_DIRS}
    FOLDER "Unit Tests/API"
  )
  add_test( NAME ${TestName} COMMAND ${TestName}-test )
  
#  target_link_directories( ${TestName}-test-dyn PRIVATE ${Boost_LIBRARY_DIRS} )
  target_link_libraries( ${TestName}-test-dyn cool.ng.dynamic ${Boost_LIBRARIES} )
  target_include_directories( ${TestName}-test-dyn SYSTEM PRIVATE ${Boost_INCLUDE_DIR} )
  target_include_directories( ${TestName}-test-dyn PRIVATE ${COOL_NG_HOME}/tests/unit )
  target_compile_definitions( ${TestName}-test-dyn PRIVATE BOOST_TEST_DYN_LINK )
  set_target_properties( ${TestName}-test-dyn PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${COOL_NG_TEST_DIR}
    LINK_DIRECTORIES  ${Boost_LIBRARY_DIRS}
    FOLDER "Unit Tests/API"
  )
  add_test( NAME ${TestName}-dyn COMMAND ${TestName}-test-dyn )
endmacro()

### ############### Find Boos.UnitTest library

if ( WINDOWS )
  if( NOT DEFINED BOOST_ROOT )
    set( BOOST_ROOT "c:/local/boost/boost_1_62_0" )
    message( WARNING "-- BOOST_ROOT was not set, will try at ${BOOST_ROOT}" )
  endif()
  
  set( BOOST_LIBRARYDIR "${BOOST_ROOT}/lib64-msvc-14.0")
endif()

# unit tests require Boost.Test library
set( Boost_USE_STATIC_LIBS       OFF )
set( Boost_USE_MULTITHREADED      ON )
set( Boost_USE_STATIC_RUNTIME    OFF )

find_package( Boost 1.58.0 COMPONENTS unit_test_framework)

if( NOT Boost_FOUND )
  message( WARNING "Boost.Test package is required to build unit tests. Will disable unit tests compilation and proceed without unit testst" )
  set( COOL_NG_UNIT_TESTS false )
else()
  message("-- Boost found, version ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}.")
  message("      Include directory: ${Boost_INCLUDE_DIR}")
  message("      Library directory: ${Boost_LIBRARY_DIRS}")
endif()

if( COOL_NG_BUILD_UNIT_TESTS )

  enable_testing()

  # --- unit tests for internal header stuff
  foreach( ut ${HEADER_ONLY_UNIT_TESTS} )
    header_unit_test( ${ut} ${${ut}_SRCS} )
  endforeach()

  # --- unit tests for internal stuff using static library
  foreach( ut ${LIBRARY_UNIT_TESTS} )
    internal_unit_test( ${ut} ${${ut}_SRCS} )
  endforeach()

  # --- API unit tests using both dynamic and static library
  foreach( ut ${API_UNIT_TESTS} )
    api_unit_test( ${ut} ${${ut}_SRCS} )
  endforeach()



endif()
