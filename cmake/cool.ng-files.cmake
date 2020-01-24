macro (append Var)
  set (${Var} ${${Var}} ${ARGN})
endmacro()

macro( add_to_list ListType )
  if ( NOT (${ListType} MATCHES "ALL" OR ${ListType} MATCHES "BUILD") )
      message( FATAL_ERROR "Unknown list type '${ListType}'")
   endif()
  append( COOL_NG_${ListType}_FILES ${ARGN} )
endmacro()

macro( add_build_files )
  add_to_list( BUILD ${ARGN} )
endmacro()

macro( add_all_files )
  add_to_list( ALL ${ARGN} )
endmacro()

##### Common Files

# --- outputs
set( COOL_NG_COMPONENT_INCLUDE_DIRECTORIES ${COOL_NG_HOME}/include )


set( COOL_NG_API_OTHER_HEADERS
    include/cool/ng/impl/platform.h
)
set( COOL_NG_API_ADDITIONAL_DOC_FILES
  doc/api/mainpage.dox
)

# --- ------------------------------------------------
# --- ------------------------------------------------
# ---
# --- Utilities Module
# ---
# --- ------------------------------------------------
# --- ------------------------------------------------
set( MODULE_DOCUMENTED_API_HEADERS
  include/cool/ng/bases.h
  include/cool/ng/binary.h
)
set( MODULE_OTHER_API_HEADERS
  include/cool/ng/impl/binary.h
)
set( MODULE_COMMON_SRCS
  ${COOL_NG_HOME}/lib/src/bases.cpp
  ${COOL_NG_HOME}/lib/src/binary.cpp
)
set( MODULE_DOC_FILES
  doc/api/module-util.dox
)
set( MODULE_FILES
  ${MODULE_DOCUMENTED_API_HEADERS}
  ${MODULE_OTHER_API_HEADERS}
  ${MODULE_COMMON_SRCS}
)
add_build_files( ${MODULE_FILES} )
add_all_files( ${MODULE_FILES} )
append( COOL_NG_DOCUMENTED_API_HEADERS   ${MODULE_DOCUMENTED_API_HEADERS} )
append( COOL_NG_API_OTHER_HEADERS        ${MODULE_OTHER_API_HEADERS} )
append( COOL_NG_LIB_COMMON_SRCS          ${MODULE_COMMON_SRCS} )
append( COOL_NG_API_ADDITIONAL_DOC_FILES ${MODULE_DOC_FILES} )

source_group("Utility" FILES ${MODULE_DOCUMENTED_API_HEADERS} ${MODULE_COMMON_SRCS} ${MODULE_DOC_FILES} )
source_group("Utility\\Impl" FILES ${MODULE_OTHER_API_HEADERS} )
# --- ------------------------------------------------
# --- ------------------------------------------------
# ---
# --- Error Handling Module
# ---
# --- ------------------------------------------------
# --- ------------------------------------------------
set( MODULE_DOCUMENTED_API_HEADERS
  include/cool/ng/exception.h
  include/cool/ng/error.h
)
set( MODULE_OTHER_API_HEADERS
)
set( MODULE_COMMON_SRCS
  ${COOL_NG_HOME}/lib/src/exception.cpp
  ${COOL_NG_HOME}/lib/src/error.cpp
)
set( MODULE_DOC_FILES
  doc/api/module-errh.dox
)

add_build_files( ${MODULE_DOCUMENTED_API_HEADERS} ${MODULE_OTHER_API_HEADERS} ${MODULE_COMMON_SRCS} )
add_all_files( ${MODULE_DOCUMENTED_API_HEADERS} ${MODULE_OTHER_API_HEADERS} ${MODULE_COMMON_SRCS} )
append( COOL_NG_DOCUMENTED_API_HEADERS   ${MODULE_DOCUMENTED_API_HEADERS} )
append( COOL_NG_API_OTHER_HEADERS        ${MODULE_OTHER_API_HEADERS} )
append( COOL_NG_LIB_COMMON_SRCS          ${MODULE_COMMON_SRCS} )
append( COOL_NG_API_ADDITIONAL_DOC_FILES ${MODULE_DOC_FILES} )

source_group("Error Handling" FILES ${MODULE_DOCUMENTED_API_HEADERS} ${MODULE_COMMON_SRCS} ${MODULE_DOC_FILES} )

# --- ------------------------------------------------
# --- ------------------------------------------------
# ---
# --- IP Address Module
# ---
# --- ------------------------------------------------
# --- ------------------------------------------------
set( MODULE_DOCUMENTED_API_HEADERS
  include/cool/ng/ip_address.h
)
set( MODULE_OTHER_API_HEADERS
  include/cool/ng/impl/ip_address.h
)
set( MODULE_COMMON_SRCS
  ${COOL_NG_HOME}/lib/src/ip_address.cpp
)
set( MODULE_DOC_FILES
  doc/api/module-ip.dox
)

add_build_files( ${MODULE_DOCUMENTED_API_HEADERS} ${MODULE_OTHER_API_HEADERS} ${MODULE_COMMON_SRCS} )
add_all_files( ${MODULE_DOCUMENTED_API_HEADERS} ${MODULE_OTHER_API_HEADERS} ${MODULE_COMMON_SRCS} )
append( COOL_NG_DOCUMENTED_API_HEADERS   ${MODULE_DOCUMENTED_API_HEADERS} )
append( COOL_NG_API_OTHER_HEADERS        ${MODULE_OTHER_API_HEADERS} )
append( COOL_NG_LIB_COMMON_SRCS          ${MODULE_COMMON_SRCS} )
append( COOL_NG_API_ADDITIONAL_DOC_FILES ${MODULE_DOC_FILES} )

source_group("IP Address" FILES ${MODULE_DOCUMENTED_API_HEADERS} ${MODULE_COMMON_SRCS} ${MODULE_DOC_FILES} )
source_group("IP Address\\Impl" FILES ${MODULE_OTHER_API_HEADERS} )

# --- ------------------------------------------------
# --- ------------------------------------------------
# ---
# --- Async Module
# ---
# --- ------------------------------------------------
# --- ------------------------------------------------

# common stuff
set( MODULE_DOCUMENTED_API_HEADERS
  include/cool/ng/async/task.h
  include/cool/ng/async/runner.h
  include/cool/ng/async/event_sources.h
  include/cool/ng/async/net/server.h
  include/cool/ng/async/net/stream.h
)
set( MODULE_OTHER_API_HEADERS
  include/cool/ng/async.h
  include/cool/ng/traits.h
  include/cool/ng/impl/async/task_traits.h
  include/cool/ng/impl/async/context.h
  include/cool/ng/impl/async/task.h
  include/cool/ng/impl/async/simple_impl.h
  include/cool/ng/impl/async/sequential_impl.h
  include/cool/ng/impl/async/intercept_impl.h
  include/cool/ng/impl/async/conditional_impl.h
  include/cool/ng/impl/async/repeat_impl.h
  include/cool/ng/impl/async/loop_impl.h
  include/cool/ng/impl/async/event_sources_types.h
  include/cool/ng/impl/async/net_server.h
  include/cool/ng/impl/async/net_stream.h
)
set( MODULE_COMMON_SRCS
  ${COOL_NG_HOME}/lib/src/async/runner.cpp
)
set( MODULE_DOC_FILES
  doc/api/module-async.dox
)

add_build_files( ${MODULE_DOCUMENTED_API_HEADERS} ${MODULE_OTHER_API_HEADERS} ${MODULE_COMMON_SRCS} )
add_all_files( ${MODULE_DOCUMENTED_API_HEADERS} ${MODULE_OTHER_API_HEADERS} ${MODULE_COMMON_SRCS} )
append( COOL_NG_DOCUMENTED_API_HEADERS   ${MODULE_DOCUMENTED_API_HEADERS} )
append( COOL_NG_API_OTHER_HEADERS        ${MODULE_OTHER_API_HEADERS} )
append( COOL_NG_LIB_COMMON_SRCS          ${MODULE_COMMON_SRCS} )
append( COOL_NG_API_ADDITIONAL_DOC_FILES ${MODULE_DOC_FILES} )

source_group("Async" FILES ${MODULE_DOCUMENTED_API_HEADERS} ${MODULE_COMMON_SRCS} ${MODULE_DOC_FILES} )
source_group("Async\\Impl" FILES ${MODULE_OTHER_API_HEADERS} )


# --- run_queue comes in three flavors, depending on the underlying queue technology

set( COOL_NG_GCD_RUN_QUEUE_DIR   ${COOL_NG_HOME}/lib/src/async/run_queue/gcd )
set( COOL_NG_DEQUE_RUN_QUEUE_DIR ${COOL_NG_HOME}/lib/src/async/run_queue/deque )
set( COOL_NG_WINCP_RUN_QUEUE_DIR ${COOL_NG_HOME}/lib/src/async/run_queue/wincp )

if ( COOL_TASK_RUNNER_IMPL STREQUAL "GCD_TARGET_QUEUE" )
  message("--- Task runner will use GCD scheduler with the target queue")
  set ( COOL_NG_RUN_QUEUE_DIR ${COOL_NG_GCD_RUN_QUEUE_DIR} )
elseif ( COOL_TASK_RUNNER_IMPL STREQUAL "GCD_DEQUE" )
  message("--- Task runner will use GCD scheduler with std::deque task queue")
  set ( COOL_NG_RUN_QUEUE_DIR ${COOL_NG_DEQUE_RUN_QUEUE_DIR} )
elseif  ( COOL_TASK_RUNNER_IMPL STREQUAL "WIN_COMPLETION_PORT" )
  message("--- Task runner will use Windows Completion Ports with the thread pool")
  set ( COOL_NG_RUN_QUEUE_DIR ${COOL_NG_WINCP_RUN_QUEUE_DIR} )
else()
  message( FATAL_ERROR "Unknown run_queue platform '${COOL_TASK_RUNNER_IMPL}', cannot proceed" )
endif()

set( COOL_NG_RUN_QUEUE_SRCS ${COOL_NG_RUN_QUEUE_DIR}/run_queue.cpp )
set( COOL_NG_RUN_QUEUE_HEADERS ${COOL_NG_RUN_QUEUE_DIR}/run_queue.h )
if ( TASK_RUNNER_IMPL STREQUAL "WIN_COMPLETION_PORT" )
  set( COOL_NG_RUN_QUEUE_HEADERS ${COOL_NG_RUN_QUEUE_HEADERS} ${COOL_NG_RUN_QUEUE_DIR}/critical_section.h )
endif()

add_build_files( ${COOL_NG_RUN_QUEUE_HEADERS} ${COOL_NG_RUN_QUEUE_SRCS} )
add_all_files(
  ${COOL_NG_GCD_RUN_QUEUE_DIR}/run_queue.h
  ${COOL_NG_DEQUE_RUN_QUEUE_DIR}/run_queue.h
  ${COOL_NG_WINCP_RUN_QUEUE_DIR}/run_queue.h
  ${COOL_NG_WINCP_RUN_QUEUE_DIR}/critical_section.h
  ${COOL_NG_GCD_RUN_QUEUE_DIR}/run_queue.cpp
  ${COOL_NG_DEQUE_RUN_QUEUE_DIR}/run_queue.cpp
  ${COOL_NG_WINCP_RUN_QUEUE_DIR}/run_queue.cpp
)

# set the correct include path for runner implementation headers
include_directories( ${COOL_NG_RUN_QUEUE_DIR} )

# --- event sources have two implementations, one for GCD based run queue and the other for the Windows Thread Pool based run queue

set (COOL_NG_GCD_EVENT_SOURCES_SRCS       ${COOL_NG_HOME}/lib/src/async/gcd/event_sources.cpp )
set (COOL_NG_GCD_EVENT_SOURCES_HEADERS    ${COOL_NG_HOME}/lib/src/async/gcd/event_sources.h )

set (COOL_NG_WINCP_EVENT_SOURCES_SRCS     ${COOL_NG_HOME}/lib/src/async/wincp/event_sources.cpp )
set (COOL_NG_WINCP_EVENT_SOURCES_HEADERS  ${COOL_NG_HOME}/lib/src/async/wincp/event_sources.h )

#if ( NOT WINDOWS )
#  set (COOL_NG_RUN_QUEUE_FILES ${COOL_NG_GCD_RUN_QUEUE_SRCS} ${COOL_NG_GCD_RUN_QUEUE_HEADERS})
#  set (COOL_NG_EVENT_SOURCES_FILES ${COOL_NG_GCD_EVENT_SOURCES_SRCS} ${COOL_NG_GCD_EVENT_SOURCES_HEADERS})
#else()
#  set (COOL_NG_RUN_QUEUE_FILES ${COOL_NG_WINCP_RUN_QUEUE_SRCS} ${COOL_NG_WINCP_RUN_QUEUE_HEADERS})
#  set (COOL_NG_EVENT_SOURCES_FILES ${COOL_NG_WINCP_EVENT_SOURCES_SRCS} ${COOL_NG_WINCP_EVENT_SOURCES_HEADERS})
#endif()

add_all_files(
  ${COOL_NG_GCD_EVENT_SOURCES_HEADERS}
  ${COOL_NG_GCD_EVENT_SOURCES_SRCS}
  ${COOL_NG_WINCP_EVENT_SOURCES_HEADERS}
  ${COOL_NG_WINCP_EVENT_SOURCES_SRCS}
) 

source_group("Async\\Run Queue\\Gcd" FILES ${COOL_NG_GCD_RUN_QUEUE_DIR}/run_queue.h ${COOL_NG_GCD_RUN_QUEUE_DIR}/run_queue.cpp )
source_group("Async\\Run Queue\\Deque" FILES ${COOL_NG_DEQUE_RUN_QUEUE_DIR}/run_queue.h ${COOL_NG_DEQUE_RUN_QUEUE_DIR}/run_queue.cpp )
source_group("Async\\Run Queue\\Wincp" FILES ${COOL_NG_WINCP_RUN_QUEUE_DIR}/run_queue.h ${COOL_NG_WINCP_RUN_QUEUE_DIR}/critical_section.h ${COOL_NG_WINCP_RUN_QUEUE_DIR}/run_queue.cpp )
source_group("Async\\Event Sources\\Gcd" FILES ${COOL_NG_GCD_EVENT_SOURCES_HEADERS} ${COOL_NG_GCD_EVENT_SOURCES_SRCS} )
source_group("Async\\Event Sources\\Wincp" FILES ${COOL_NG_WINCP_EVENT_SOURCES_HEADERS} ${COOL_NG_WINCP_EVENT_SOURCES_SRCS} )

# --- End of modules

set( COOL_NG_API_HEADERS
  ${COO_NG_DOCUMENTED_API_HEADERS}
  ${COOL_NG_API_OTHER_HEADERS}
)


set( COOL_NG_API_DOCUMENTATION_FILES
  ${COO_NG_DOCUMENTED_API_HEADERS}
  ${COOL_NG_API_ADDITIONAL_DOC_FILES}
)

add_custom_target("Modules" SOURCES
  ${COOL_NG_ALL_FILES}
)









set( COOL_NG_PLATFORM_SRCS    ${COOL_NG_RUN_QUEUE_SRCS} )
set( COOL_NG_PLATFORM_HEADERS ${COOL_NG_RUN_QUEUE_HEADERS} )

set( COOL_NG_LIB_HEADERS ${COOL_NG_API_HEADERS} ${COOL_NG_IMPL_COMMON_HEADERS} ${COOL_NG_PLATFORM_HEADERS} )
set( COOL_NG_LIB_SRCS ${COOL_NG_LIB_COMMON_SRCS} ${COOL_NG_PLATFORM_SRCS} )

set( COOL_NG_LIB_FILES ${COOL_NG_LIB_HEADERS} ${COOL_NG_LIB_SRCS} )


# --- library file names names
if( NOT WINDOWS )
  set( COOL_NG_LIB_STATIC      cool.ng)            # file names
  set( COOL_NG_LIB_DYNAMIC     cool.ng)
else()
  set( COOL_NG_LIB_STATIC      libcool.ng-vc${COOL_NG_MSVC})
  set( COOL_NG_LIB_DYNAMIC     cool.ng-vc${COOL_NG_MSVC})
endif()
