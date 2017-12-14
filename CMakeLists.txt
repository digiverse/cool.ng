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

# --- top of source tree
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

# --- enable/disable documentation build
if( NOT DEFINED COOL_NG_BUILD_DOC )
  set( COOL_NG_BUILD_DOC true )
endif()

# --- library and binary directories
if( NOT DEFINED COOL_NG_LIB_DIR )
  set( COOL_NG_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib)
endif()

if( NOT DEFINED COOL_NG_BIN_DIR)
  set( COOL_NG_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
endif()

if( NOT DEFINED COOL_NG_DOC_DIR)
  set( COOL_NG_DOC_DIR ${CMAKE_CURRENT_BINARY_DIR}/doc)
endif()

# --- test programs location
if( COOL_NG_UNIT_TESTS )
  if( NOT DEFINED COOL_NG_TEST_DIR )
    set( COOL_NG_TEST_DIR ${COOL_NG_BIN_DIR} )
  endif()
endif()

# --- Boost root, needed for unit tests
if ( NOT DEFINED BOOST_ROOT )
  if ( COOL_NG_UNIT_TESTS )
    message("BOOST_ROOT is not set, will use internal hardcoded location")
  endif()
endif()

# --- whether to compile development library versions or not
if( NOT DEFINED COOL_NG_DEV_LIBS )
  set ( COOL_NG_DEV_LIBS true)
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
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC -fbracket-depth=10000" )
  set( CMAKE_EXE_LINKER_FLAGS "-w" )
  add_definitions( "-DOS_OBJECT_USE_OBJC=0" )
  add_definitions( "-DCOOL_ASYNC_PLATFORM_GCD" )

elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

# --- Linux
  set( BUILD_TARGET LINUX_TARGET )
  set( LINUX true )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC" )
  add_definitions("-DCOOL_ASYNC_PLATFORM_GCD")
  set( COOL_NG_PLATFORM_LIBRARIES pthread dispatch)
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
  add_definitions("-DCOOL_ASYNC_PLATFORM_WINCP")
  set( COOL_NG_PLATFORM_LIBRARIES DbgHelp.dll Ws2_32.dll)

  if( ${MSVC_VERSION} MATCHES "1800")
    set( COOL_MSVC "12" )
  elseif( ${MSVC_VERSION} MATCHES "1900" )
    set( COOL_MSVC "14" )
  elseif( ${MSVC_VERSION} MATCHES "1910" )
    set( COOL_MSVC "15" )
  else()
    message( FATAL "unsupported visual studio compiler version ${MSVC_VERSION}" )
  endif()
else()

# --- unknown target
  message( FATAL "unsupported platform" )

endif()

add_definitions( "-D${BUILD_TARGET}" )
include_directories(${COOL_NG_HOME}/include ${COOL_NG_HOME}/lib ${COOL_NG_HOME}/lib/include)

# ### ##################################################
# ###
# ### Library names
# ###
# ### ##################################################

if( NOT WINDOWS )
  set( COOL_NG_LIB_STATIC      cool.ng)
  set( COOL_NG_LIB_DYNAMIC     cool.ng)
  set( COOL_NG_LIB_STATIC_DEV  cool.ng-dev)
  set( COOL_NG_LIB_DYNAMIC_DEV cool.ng-dev)
else()
  set( COOL_NG_LIB_STATIC      libcool.ng-vc${COOL_MSVC})
  set( COOL_NG_LIB_DYNAMIC     cool.ng-vc${COOL_MSVC})
  set( COOL_NG_LIB_STATIC_DEV  libcool.ng-dev-vc${COOL_MSVC})
  set( COOL_NG_LIB_DYNAMIC_DEV cool.ng-dev-vc${COOL_MSVC})
endif()

# ### ##################################################
# ###
# ### Library source files
# ###
# ### ##################################################


set( COOL_NG_API_HEADERS
    include/cool/ng/async.h
    include/cool/ng/error.h
    include/cool/ng/exception.h
    include/cool/ng/traits.h
    include/cool/ng/bases.h
    include/cool/ng/ip_address.h
    include/cool/ng/binary.h
    include/cool/ng/async/task.h
    include/cool/ng/async/runner.h
    include/cool/ng/async/event_sources.h
    include/cool/ng/async/net/server.h
    include/cool/ng/async/net/stream.h
)

set( COOL_NG_IMPL_HEADERS
    include/cool/ng/impl/platform.h
    include/cool/ng/impl/ip_address.h
    include/cool/ng/impl/async/task_traits.h
    include/cool/ng/impl/async/context.h
    include/cool/ng/impl/async/task.h
    include/cool/ng/impl/async/simple_impl.h
    include/cool/ng/impl/async/sequential_impl.h
    include/cool/ng/impl/async/intercept_impl.h
    include/cool/ng/impl/async/conditional_impl.h
    include/cool/ng/impl/async/repeat_impl.h
    include/cool/ng/impl/async/loop_impl.h
    include/cool/ng/impl/async/event_sources.h
    include/cool/ng/impl/async/net_types.h
    include/cool/ng/impl/async/net_server.h
    include/cool/ng/impl/async/net_stream.h
)

# ### ##################################################
# ###
# ### Cool library
# ###
# ### ##################################################

set( COOL_NG_LIB_HEADERS
  lib/include/lib/async/executor.h
)

set( COOL_NG_LIB_SRCS
  lib/src/bases.cpp
  lib/src/exception.cpp
  lib/src/error.cpp
  lib/src/ip_address.cpp
  lib/src/async/runner.cpp
  lib/src/async/event_sources.cpp
)

# --- executor sources
set (COOL_NG_GCD_EVENT_SOURCES_SRCS    lib/src/async/gcd/event_sources.cpp )
set (COOL_NG_GCD_EVENT_SOURCES_HEADERS lib/src/async/gcd/event_sources.h )
set (COOL_NG_GCD_EXECUTOR_SRCS         lib/src/async/gcd/executor.cpp )
set (COOL_NG_GCD_EXECUTOR_HEADERS      lib/src/async/gcd/executor.h )

set (COOL_NG_WINCP_EVENT_SOURCES_SRCS    lib/src/async/wincp/event_sources.cpp )
set (COOL_NG_WINCP_EVENT_SOURCES_HEADERS lib/src/async/wincp/event_sources.h )
set (COOL_NG_WINCP_EXECUTOR_SRCS         lib/src/async/wincp/executor.cpp)
set (COOL_NG_WINCP_EXECUTOR_HEADERS      lib/src/async/wincp/executor.h)

if ( NOT WINDOWS )
  set (COOL_NG_EXECUTOR_FILES ${COOL_NG_GCD_EXECUTOR_SRCS} ${COOL_NG_GCD_EXECUTOR_HEADERS})
  set (COOL_NG_EVENT_SOURCES_FILES ${COOL_NG_GCD_EVENT_SOURCES_SRCS} ${COOL_NG_GCD_EVENT_SOURCES_HEADERS})
else()
  set (COOL_NG_EXECUTOR_FILES ${COOL_NG_WINCP_EXECUTOR_SRCS} ${COOL_NG_WINCP_EXECUTOR_HEADERS})
  set (COOL_NG_EVENT_SOURCES_FILES ${COOL_NG_WINCP_EVENT_SOURCES_SRCS} ${COOL_NG_WINCP_EVENT_SOURCES_HEADERS})
endif()

set( COOL_NG_GCD_IMPL_HEADERS
  ${COOL_NG_GCD_EXECUTOR_HEADERS}
  ${COOL_NG_GCD_EVENT_SOURCES_HEADERS}
)

set( COOL_NG_GCD_IMPL_SRCS
  ${COOL_NG_GCD_EXECUTOR_SRCS}
  ${COOL_NG_GCD_EVENT_SOURCES_SRCS}
)

set( COOL_NG_WINCP_IMPL_HEADERS
  ${COOL_NG_WINCP_EXECUTOR_HEADERS}
  ${COOL_NG_WINCP_EVENT_SOURCES_HEADERS}
)
set( COOL_NG_WINCP_IMPL_SRCS
  ${COOL_NG_WINCP_EXECUTOR_SRCS}
  ${COOL_NG_WINCP_EVENT_SOURCES_SRCS}
)

# -- cool library sources
set( COOL_NG_LIB_FILES ${COOL_NG_LIB_HEADERS} ${COOL_NG_LIB_SRCS} )
if( WINDOWS )
  set( COOL_NG_LIB_FILES ${COOL_NG_LIB_FILES} ${COOL_NG_WINCP_IMPL_HEADERS} ${COOL_NG_WINCP_IMPL_SRCS} )
else()
  set( COOL_NG_LIB_FILES ${COOL_NG_LIB_FILES} ${COOL_NG_GCD_IMPL_HEADERS} ${COOL_NG_GCD_IMPL_SRCS} )
endif()
message(" ====== ${COOL_NG_LIB_FILES}")

# --- developmen libraries

add_library( cool.ng-dyn-dev SHARED ${COOL_NG_LIB_FILES} )
add_library( cool.ng-dev STATIC ${COOL_NG_LIB_FILES} )
target_compile_definitions(cool.ng-dev PUBLIC -DCOOL_NG_BUILD -DCOOL_NG_STATIC_LIBRARY)
target_compile_definitions(cool.ng-dyn-dev PUBLIC -DCOOL_NG_BUILD)

# --- now set file names and locations
if( NOT WINDOWS )
  set_target_properties( cool.ng-dyn-dev PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "${COOL_NG_LIB_DYNAMIC_DEV}"
    LIBRARY_OUTPUT_DIRECTORY "${COOL_NG_LIB_DIR}"
  )
  set_target_properties( cool.ng-dev PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "${COOL_NG_LIB_STATIC_DEV}"
    ARCHIVE_OUTPUT_DIRECTORY "${COOL_NG_LIB_DIR}"
  )
else()
  set_target_properties( cool.ng-dyn-dev PROPERTIES
    PREFIX ""
    OUTPUT_NAME "${COOL_NG_LIB_DYNAMIC_DEV}"
    RUNTIME_OUTPUT_DIRECTORY "${COOL_NG_BIN_DIR}"
  )
  target_link_libraries( cool.ng-dyn-dev ${COOL_NG_PLATFORM_LIBRARIES} )
  set_target_properties( cool.ng-dev PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "${COOL_NG_LIB_STATIC_DEV}"
    ARCHIVE_OUTPUT_DIRECTORY "${COOL_NG_LIB_DIR}"
  )
  target_link_libraries( cool.ng-dev ${COOL_NG_PLATFORM_LIBRARIES} )
endif()

# ### ##################################################
# ###
# ### Doxygen documentation
# ###
# ### ##################################################


if (COOL_NG_BUILD_DOC)

  find_package(Doxygen)
  if (NOT Doxygen_FOUND)
    message(WARNING "Doxygen package not found, will not generate documentation")
  else ()
    make_directory (${COOL_NG_DOC_DIR})
    configure_file(
      ${COOL_NG_HOME}/cool.ng.dox.in
      ${COOL_NG_DOC_DIR}/cool.ng.dox
      @ONLY
    )
    add_custom_command(
      OUTPUT ${COOL_NG_DOC_DIR}/doc/html/index.html
      COMMAND ${DOXYGEN_EXECUTABLE} ${COOL_NG_DOC_DIR}/cool.ng.dox
      DEPENDS ${COOL_NG_API_HEADERS} ${COOL_NG_HOME}/mainpage.dox
      WORKING_DIRECTORY ${COOL_NG_DOC_DIR}
    )
    add_custom_target(doc ALL DEPENDS ${COOL_NG_DOC_DIR}/doc/html/index.html)
  endif()

endif()

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
  simple_task
  sequential_task
  intercept_task
  conditional_task
  repeat_task
  loop_task
  ip_address
  es_reader
  es_timer
)

set( traits_SRCS tests/unit/traits/traits.cpp )
set( task-traits_SRCS tests/unit/traits/task_traits.cpp )
set( executor_SRCS tests/unit/executor/executor.cpp )
set( simple_task_SRCS tests/unit/task/simple_task.cpp )
set( sequential_task_SRCS tests/unit/task/sequential_task.cpp )
set( intercept_task_SRCS tests/unit/task/intercept_task.cpp )
set( conditional_task_SRCS tests/unit/task/conditional_task.cpp )
set( repeat_task_SRCS tests/unit/task/repeat_task.cpp )
set( loop_task_SRCS tests/unit/task/loop_task.cpp )
set( ip_address_SRCS tests/unit/net/ip_address.cpp )
set( es_reader_SRCS tests/unit/event_sources/es_reader.cpp )
set( es_timer_SRCS tests/unit/event_sources/es_timer.cpp )

macro(header_unit_test TestName)
  add_executable( ${TestName}-test ${ARGN} )
  if( WINDOWS )
    target_link_libraries( ${TestName}-test ${COOL_NG_PLATFORM_LIBRARIES} )
  else()
    # --- on windows we use autolink feature
    target_link_libraries( ${TestName}-test ${Boost_LIBRARIES} ${COOL_NG_PLATFORM_LIBRARIES} )
  endif()
  set_target_properties( ${TestName}-test PROPERTIES
      COMPILE_FLAGS -DBOOST_TEST_DYN_LINK
      RUNTIME_OUTPUT_DIRECTORY ${COOL_NG_TEST_DIR}
      FOLDER "Unit Tests/Header"
  )
  add_test( NAME ${TestName} COMMAND ${TestName}-test )
endmacro()

macro(internal_unit_test TestName)
  add_executable( ${TestName}-test ${ARGN} )
  target_link_libraries(${TestName}-test cool.ng-dev)

  if( WINDOWS )
    target_link_libraries( ${TestName}-test ${COOL_NG_PLATFORM_LIBRARIES} )
  else()
    # --- on windows we use autolink feature
    target_link_libraries( ${TestName}-test ${Boost_LIBRARIES} ${COOL_NG_PLATFORM_LIBRARIES} )
  endif()
  target_compile_definitions(${TestName}-test PUBLIC "-DCOOL_NG_STATIC_LIBRARY")
  set_target_properties( ${TestName}-test PROPERTIES
    COMPILE_FLAGS -DBOOST_TEST_DYN_LINK
    RUNTIME_OUTPUT_DIRECTORY ${COOL_NG_TEST_DIR}
    FOLDER "Unit Tests/Internal"
  )
  add_test( NAME ${TestName} COMMAND ${TestName}-test )
endmacro()

macro(api_unit_test TestName)
  add_executable( ${TestName}-test ${ARGN} )
  target_link_libraries(${TestName}-test cool.ng-dyn-dev)

  if( WINDOWS )
    target_link_libraries( ${TestName}-test ${COOL_NG_PLATFORM_LIBRARIES} )
  else()
    # --- on windows we use autolink feature
    target_link_libraries( ${TestName}-test ${Boost_LIBRARIES} ${COOL_NG_PLATFORM_LIBRARIES} )
  endif()
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
  link_directories( ${Boost_LIBRARY_DIRS} )

  enable_testing()
  # --- unit tests for internal header stuff
  foreach( ut ${HEADER_ONLY_UNIT_TESTS} )
    header_unit_test( ${ut} ${${ut}_SRCS} )
    set( HEADER_ONLY_UNIT_TESTS_SOURCES ${HEADER_ONLY_UNIT_TESTS_SOURCES} ${${ut}_SRCS} )
  endforeach()

  # --- unit tests for internal stuff using static library
  foreach( ut ${LIBRARY_UNIT_TESTS} )
    internal_unit_test( ${ut} ${${ut}_SRCS} )
    set( LIBRARY_UNIT_TESTS_SOURCES ${LIBRARY_UNIT_TESTS_SOURCES} ${${ut}_SRCS} )
  endforeach()

  # --- API unit tests using dynamic library
  foreach( ut ${API_UNIT_TESTS} )
    api_unit_test( ${ut} ${${ut}_SRCS} )
    set( LIBRARY_UNIT_TESTS_SOURCES ${LIBRARY_UNIT_TESTS_SOURCES} ${${ut}_SRCS} )
  endforeach()

endif()

# ### ##################################################
# ###
# ### Source organization for IDEs
# ###
# ### ##################################################

source_group( "API Header Files" FILES ${COOL_NG_API_HEADERS} )
source_group( "Impl Header Files" FILES ${COOL_NG_IMPL_HEADERS} )
source_group( "Header Unit Tests" FILES ${HEADER_ONLY_UNIT_TESTS_SOURCES} )
source_group( "Library Unit Tests" FILES ${LIBRARY_UNIT_TESTS_SOURCES} )
source_group( "GCD Specific Files" FILES ${COOL_NG_GCD_IMPL_HEADERS} ${COOL_NG_GCD_IMPL_SRCS} )
source_group( "Windows Specific Files" FILES ${COOL_NG_WINCP_IMPL_HEADERS} ${COOL_NG_WINCP_IMPL_SRCS} )
source_group( "Library Files" FILES ${COOL_NG_LIB_HEADERS} ${COOL_NG_LIB_SRCS} )
source_group( "Documentation" FILES ${COOL_NG_HOME}/mainpage.dox )

# --- custom target listing all source files (for IDEs only)
add_custom_target("All-Sources" SOURCES
  ${COOL_NG_API_HEADERS}
  ${COOL_NG_IMPL_HEADERS}
  ${COOL_NG_GCD_IMPL_HEADERS}
  ${COOL_NG_GCD_IMPL_SRCS}
  ${COOL_NG_WINCP_IMPL_HEADERS}
  ${COOL_NG_WINCP_IMPL_SRCS}
  ${COOL_NG_LIB_HEADERS}
  ${COOL_NG_LIB_SRCS}
  ${HEADER_ONLY_UNIT_TESTS_SOURCES}
  ${LIBRARY_UNIT_TESTS_SOURCES}
  ${COOL_NG_HOME}/mainpage.dox
)

