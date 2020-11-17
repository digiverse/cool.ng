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


cmake_minimum_required(VERSION 3.5.1)           # Ubuntu 16.04 LTS as the oldest platform comes with cmake 3.5.1
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if( POLICY CMP0074 )
  cmake_policy( SET CMP0074 OLD )
endif()

include( cmake/cool.ng-platform.cmake )

# ---  silent mode will only report errors
if( NOT DEFINED COOL_NG_SILENT )
  set( COOL_NG_SILENT false )
endif()

# --- top of source tree
if( NOT DEFINED COOL_NG_HOME )
  set( COOL_NG_HOME ${CMAKE_CURRENT_SOURCE_DIR} )
endif()

# --- build directory
if( NOT DEFINED COOL_NG_BUILD_DIR )
  set( COOL_NG_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} )
endif()

# --- enable/disable and control unit tests
if( NOT DEFINED COOL_NG_BUILD_UNIT_TESTS )
  set( COOL_NG_BUILD_UNIT_TESTS true)
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
if( COOL_NG_BUILD_UNIT_TESTS )
  if( NOT DEFINED COOL_NG_TEST_DIR )
    set( COOL_NG_TEST_DIR ${COOL_NG_BIN_DIR} )
  endif()
endif()


report( "-- Generating Cool.NG build files for ${COOL_PLATFORM_TARGET} with the following parameters:" )
report( "   COOL_NG_SILENT ............. ${COOL_NG_SILENT}" )
report( "   COOL_NG_HOME ............... ${COOL_NG_HOME}" )
report( "   COOL_NG_BUILD_DIR .......... ${COOL_NG_BUILD_DIR}" )
report( "   COOL_NG_BUILD_UNIT_TESTS ... ${COOL_NG_BUILD_UNIT_TESTS}" )
report( "   COOL_NG_BUILD_DOC .......... ${COOL_NG_BUILD_DOC}" )
report( "   COOL_NG_BIN_DIR ............ ${COOL_NG_BIN_DIR}" )
report( "   COOL_NG_LIB_DIR ............ ${COOL_NG_LIB_DIR}" )
report( "   COOL_NG_DOC_DIR ............ ${COOL_NG_DOC_DIR}" )
report( "   COOL_PLATFORM_TARGET ....... ${COOL_PLATFORM_TARGET}" )
report( "   COOL_ASYNC_PLATFORM ........ ${COOL_ASYNC_PLATFORM}" )
report( "   COOL_TASK_RUNNER_IMPL ...... ${COOL_TASK_RUNNER_IMPL}" )

if( COOL_NG_BUILD_UNIT_TESTS )
  report( "   COOL_NG_TEST_DIR ........... ${COOL_NG_TEST_DIR}" )
  if( DEFINED BOOST_ROOT ) 
    report( "   BOOST_ROOT ................. ${BOOST_ROOT}" )
  else()
    report( "   BOOST_ROOT ................. not defined, will use internal default" )
  endif()
endif()

# --- logical targert for static and dynamic libraries
set ( COOL_NG_TARGET_STATIC  cool.ng.archive )    # logical targets
set ( COOL_NG_TARGET_DYNAMIC cool.ng.dynamic )

include( cmake/cool.ng-files.cmake )
include( cmake/cool.ng-library.cmake )
if( COOL_NG_BUILD_DOC )
  include( cmake/cool.ng-doc.cmake )
  cool_ng_api_doc( ${COOL_NG_HOME}/doc doc doc ${COOL_NG_API_DOCUMENTATION_FILES} )
endif()

if ( COOL_NG_BUILD_UNIT_TESTS )
  include( cmake/cool.ng-test.cmake )
endif()



