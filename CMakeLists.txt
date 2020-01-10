#
# Copyright (c) 2017 Leon Mlakar.
# Copyright (c) 2017 Digiverse d.o.o.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License. The
# license should be included in the source distribution of the Software;
# if not, you may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# The above copyright notice and licensing terms shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

project (cool.ng)
cmake_minimum_required(VERSION 3.5)

# ### ##################################################
# ###
# ### Build control variables that may get set from
# ### outside to integrate into larger projects
# ###
# ### ##################################################

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ### ##################################################
# ###
# ### Platform identification and compiler setup
# ###
# ### Note:
# ###  TASK_RUNNER_IMPL identifies the technology used
# ###  by runner to ensure FIFO behavior:
# ###    * GCD_TARGET_QUEUE uses GCD task queue with
# ###      target queue set to dispatch_get_global_queue
# ###    * GCD_DEQUE uses std::deque guarded by std::mutex
# ###      which empties into  dispatch_get_global_queue
# ###    * WIN_COMPLETION_PORT uses Windows Completion
# ###      Ports associated with the thread pool
# ###
# ###  This identifier is passed to the source code as
# ###  preprocessor macro set to the same value
# ###
# ### ##################################################

if( POLICY CMP0074 )
  cmake_policy( SET CMP0074 OLD )
 endif()

if( ${CMAKE_SYSTEM_NAME} MATCHES  "Darwin" )

# --- Apple OS/X
  set( OSX true)
  #set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC -fbracket-depth=10000" )
  add_compile_options( -g -std=c++11 -fPIC -fbracket-depth=10000 )
  set( CMAKE_EXE_LINKER_FLAGS "-w" )
  set( TASK_RUNNER_IMPL GCD_TARGET_QUEUE )
  set( COOL_PLATFORM_TARGET OSX )

elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

# --- Linux
  set( LINUX true )
  #set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC" )
  add_compile_options( -g -std=c++11 -fPIC )
  set( TASK_RUNNER_IMPL GCD_DEQUE )
  set( COOL_PLATFORM_TARGET LINUX )
elseif ( MSVC )

# --- Windows
  set( WINDOWS true )

  # CMake incorrectly appends "/machine:X86" to all linker flags when static
  # libraries are used, resulting in invalid builds:
  # > fatal error LNK1112: module machine type 'x64' conflicts with target machine type 'X86'
  # (ref. https://public.kitware.com/Bug/view.php?id=15496)
  if (CMAKE_GENERATOR MATCHES "Win64")
    set( CMAKE_EXE_LINKER_FLAGS    "/machine:X64" )
    set( CMAKE_MODULE_LINKER_FLAGS "/machine:X64" )
    set( CMAKE_SHARED_LINKER_FLAGS "/machine:X64" )
  endif()

  #set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /bigobj /Zm750 -D_SCL_SECURE_NO_WARNINGS")
  add_compile_options( /EHsc /bigobj /Zm750 )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
  set( TASK_RUNNER_IMPL WIN_COMPLETION_PORT )
  set( COOL_PLATFORM_TARGET WIN64 )

  # warnings suppression
  add_compile_options( /wd4624 /wd4091 /wd4251 /wd4275 )
else()

# --- unknown target
  message( FATAL "unsupported platform" )

endif()

add_definitions( -DPLATFORM_TARGET=${COOL_PLATFORM_TARGET} )
add_definitions( -DTASK_RUNNER_IMPL=${TASK_RUNNER_IMPL} )

include( Component.cmake )


# ### ##################################################
# ###
# ### Doxygen documentation
# ###
# ### ##################################################


#if (COOL_NG_BUILD_DOC)

#  find_package(Doxygen)
#  if (NOT Doxygen_FOUND)
#    message(WARNING "Doxygen package not found, will not generate documentation")
#  else ()
#    make_directory (${COOL_NG_DOC_DIR})
#    configure_file(
#      ${COOL_NG_HOME}/cool.ng.dox.in
#      ${COOL_NG_DOC_DIR}/cool.ng.dox
#      @ONLY
#    )
#    add_custom_command(
#      OUTPUT ${COOL_NG_DOC_DIR}/doc/html/index.html
#      COMMAND ${DOXYGEN_EXECUTABLE} ${COOL_NG_DOC_DIR}/cool.ng.dox
#      DEPENDS ${COOL_NG_API_HEADERS} ${COOL_NG_HOME}/mainpage.dox
#      WORKING_DIRECTORY ${COOL_NG_DOC_DIR}
#    )
#    add_custom_target(doc ALL DEPENDS ${COOL_NG_DOC_DIR}/doc/html/index.html)
#  endif()

#endif()

# ### ##################################################
# ###
# ### Unit tests
# ###
# ### ##################################################

set( HEADER_ONLY_UNIT_TESTS
  traits
  task-traits
)

# unit tests for library internals, require static lib owing to MS dll export/import
set( LIBRARY_UNIT_TESTS
  executor
)

# api level unit tests, will use dynamic library
set( API_UNIT_TESTS
#  simple_task
#  sequential_task
#  intercept_task
#  conditional_task
#  repeat_task
#  loop_task
  task
  ip_address
  es_reader
  es_timer
)

set( task_SRCS tests/unit/task/task_common.h tests/unit/task/task_common.cpp
  tests/unit/task/simple_task.cpp
  tests/unit/task/intercept_task.cpp
  tests/unit/task/sequential_task.cpp
  tests/unit/task/conditional_task.cpp
  tests/unit/task/repeat_task.cpp
  tests/unit/task/loop_task.cpp
)
set( traits_SRCS tests/unit/traits/traits.cpp )
set( task-traits_SRCS tests/unit/traits/task_traits.cpp )
set( executor_SRCS tests/unit/executor/executor.cpp )
#set( simple_task_SRCS tests/unit/task/simple_task.cpp )
#set( sequential_task_SRCS tests/unit/task/sequential_task.cpp )
#set( intercept_task_SRCS tests/unit/task/intercept_task.cpp )
#set( conditional_task_SRCS tests/unit/task/conditional_task.cpp )
#set( repeat_task_SRCS tests/unit/task/repeat_task.cpp )
#set( loop_task_SRCS tests/unit/task/loop_task.cpp )
set( ip_address_SRCS tests/unit/net/ip_address.cpp )
set( es_reader_SRCS tests/unit/event_sources/es_reader.cpp )
set( es_timer_SRCS tests/unit/event_sources/es_timer.cpp )
set( utilities_SRCS tests/unit/utilities/binary.cpp  tests/unit/utilities/identification.cpp)
set( error_SRCS tests/unit/error/error.cpp)

macro(header_unit_test TestName)
  add_executable( ${TestName}-test ${ARGN} )

  target_link_libraries( ${TestName}-test ${Boost_LIBRARIES} ${COOL_NG_PLATFORM_LIBRARIES} )
  set_target_properties( ${TestName}-test PROPERTIES
      COMPILE_FLAGS -DBOOST_TEST_DYN_LINK
      RUNTIME_OUTPUT_DIRECTORY ${COOL_NG_TEST_DIR}
      FOLDER "Unit Tests/Header"
  )
  target_include_directories( ${TestName}-test PUBLIC ${COOL_NG_COMPONENT_INCLUDE_DIRECTORIES})

  add_test( NAME ${TestName} COMMAND ${TestName}-test )
endmacro()

macro(internal_unit_test TestName)
  add_executable( ${TestName}-test ${ARGN} )

  target_link_libraries( ${TestName}-test cool.ng-dev ${Boost_LIBRARIES} ${COOL_NG_PLATFORM_LIBRARIES} )
  set_target_properties( ${TestName}-test PROPERTIES
    COMPILE_FLAGS -DBOOST_TEST_DYN_LINK
    RUNTIME_OUTPUT_DIRECTORY ${COOL_NG_TEST_DIR}
    FOLDER "Unit Tests/Internal"
  )
  target_compile_definitions(${TestName}-test PUBLIC "-DCOOL_NG_STATIC_LIBRARY")

  add_test( NAME ${TestName} COMMAND ${TestName}-test )
endmacro()

macro(api_unit_test TestName)
  add_executable( ${TestName}-test ${ARGN} )

  target_link_libraries( ${TestName}-test cool.ng-dyn-dev ${Boost_LIBRARIES} ${COOL_NG_PLATFORM_LIBRARIES} )
  set_target_properties( ${TestName}-test PROPERTIES
    COMPILE_FLAGS -DBOOST_TEST_DYN_LINK
    RUNTIME_OUTPUT_DIRECTORY ${COOL_NG_TEST_DIR}
    FOLDER "Unit Tests/API"
  )

  add_test( NAME ${TestName} COMMAND ${TestName}-test )
endmacro()

if( COOL_NG_UNIT_TESTS )

  if ( WINDOWS )
    if (NOT DEFINED BOOST_ROOT AND WINDOWS)
      set( BOOST_ROOT "c:/local/boost/boost_1_62_0" )
    endif()
    if ( MSVC_VERSION EQUAL "1800" )
      set( BOOST_LIBRARYDIR "${BOOST_ROOT}/lib64-msvc-12.0")
    else()
      set( BOOST_LIBRARYDIR "${BOOST_ROOT}/lib64-msvc-14.0")
    endif()
  endif()

  # unit tests require Boost.Test library
  set( Boost_USE_STATIC_LIBS       OFF )
  set( Boost_USE_MULTITHREADED      ON )
  set( Boost_USE_STATIC_RUNTIME    OFF )

  find_package( Boost 1.58.0 COMPONENTS unit_test_framework)

  if( NOT Boost_FOUND )
    message( WARNING "Boost.Test package is required to build unit tests. Will disable unit tests compilation and proceed without Boost.Test" )
    set( COOL_NG_UNIT_TESTS false )
  else()
    message("-- Boost found, version ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}.")
    message("      Include directory: ${Boost_INCLUDE_DIR}")
    message("      Library directory: ${Boost_LIBRARY_DIRS}")
  endif()
endif()

if( COOL_NG_UNIT_TESTS )

  include_directories( SYSTEM ${Boost_INCLUDE_DIR} )
  include_directories( ${COOL_NG_HOME}/tests/unit )
  link_directories( ${Boost_LIBRARY_DIRS} )

  enable_testing()
  # --- unit tests for internal header stuff
#  foreach( ut ${HEADER_ONLY_UNIT_TESTS} )
#    header_unit_test( ${ut} ${${ut}_SRCS} )
#    set( HEADER_ONLY_UNIT_TESTS_SOURCES ${HEADER_ONLY_UNIT_TESTS_SOURCES} ${${ut}_SRCS} )
#  endforeach()

  # --- unit tests for internal stuff using static library
#  foreach( ut ${LIBRARY_UNIT_TESTS} )
#    internal_unit_test( ${ut} ${${ut}_SRCS} )
#    set( LIBRARY_UNIT_TESTS_SOURCES ${LIBRARY_UNIT_TESTS_SOURCES} ${${ut}_SRCS} )
#  endforeach()

  # --- API unit tests using dynamic library
# foreach( ut ${API_UNIT_TESTS} )
#    api_unit_test( ${ut} ${${ut}_SRCS} )
#    set( LIBRARY_UNIT_TESTS_SOURCES ${LIBRARY_UNIT_TESTS_SOURCES} ${${ut}_SRCS} )
#  endforeach()

internal_unit_test( run_queue  tests/unit/run_queue/run_queue.cpp )
api_unit_test( ip_address ${ip_address_SRCS} )
api_unit_test( utilities ${utilities_SRCS} )
api_unit_test( error ${error_SRCS} )
#api_unit_test( task ${task_SRCS} )

endif()


