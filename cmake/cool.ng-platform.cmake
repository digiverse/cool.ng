#
# On MacOS/OSX:
#    - OSX set to true
#    - MACOS set to true
#    - COOL_PLATFORM_TARGET --> OSX
#    - COOL_ASYNC_PLATFORM --> COOL_ASYNC_PLATFORM_GCD
#    - COOL_TASK_RUNNER_IMPL --> GCD_TARGET_QUEUE
#
# On Windows:
#    - WINDOWS set to true
#    - COOL_PLATFORM_TARGET --> WINDOWS_TARGET
#    - COOL_ASYNC_PLATFORM --> COOL_ASYNC_PLATFORM_WINCP
#    - COOL_TASK_RUNNER_IMPL-->  WIN_COMPLETION_PORT
#
# On Linux of any flavour:
#    - LINUX set to true
#    - COOL_PLATFORM_TARGET --> LINUX
#    - COOL_ASYNC_PLATFORM --> COOL_ASYNC_PLATFORM_GCD
#    - COOL_TASK_RUNNER_IMPL -->  GCD_DEQUE
#
# On FreeBSD:
#    - FREEBSD set to true
#    - COOL_PLATFORM_TARGET --> FREEBSD
#    - COOL_ASYNC_PLATFORM --> COOL_ASYNC_PLATFORM_GCD
#    - COOL_TASK_RUNNER_IMPL -->  GCD_DEQUE
#
# It also sets COOL_NG_COMPILER_OPTIONS and COOL_NG_PLATFORM_LIBRARIES correctly for platform
# It may set COOL_NG_ADDITIONAL_SYS_INCLUDES and COOL_NG_ADDITIONAL_SYS_LIBDIRS if necessary

macro(report Level)
  if( Level STREQUAL "FATAL" )
    message( FATAL_ERROR "${ARGN}")
  elseif( LEVEL STREQUAL "ERROR" )
    message( SEND_ERROR "${ARGN}" )
  else()
    if( NOT COOL_NG_SILENT )
      if (Level STREQUAL "WARNING" )
	message( WARNING "${ARGN}" )
      else()
	message( "${Level}${ARGN}" )
      endif()
    endif()
  endif()
endmacro()
	

if( ${CMAKE_SYSTEM_NAME} MATCHES  "Darwin" )

# --- Apple OS/X
  set( OSX true)
  set( MACOS true )
  set( COOL_PLATFORM_TARGET OSX_TARGET )
  set( COOL_ASYNC_PLATFORM COOL_ASYNC_PLATFORM_GCD )
  set( COOL_TASK_RUNNER_IMPL GCD_TARGET_QUEUE )

  # platform libraries to link
  set( COOL_NG_PLATFORM_LIBRARIES )
  set( COOL_NG_COMPILER_OPTIONS -g -std=c++11 -fPIC -fbracket-depth=10000 )

elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

# --- Linux
  set( LINUX true )
  set( COOL_PLATFORM_TARGET LINUX_TARGET )
  set( COOL_ASYNC_PLATFORM COOL_ASYNC_PLATFORM_GCD )
  set( COOL_TASK_RUNNER_IMPL GCD_DEQUE )
  
  # platform libraries to link
  set( COOL_NG_PLATFORM_LIBRARIES pthread dispatch )
  set( COOL_NG_COMPILER_OPTIONS -g -std=c++11 -fPIC)

elseif( ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD" )

  set( FREEBSD true )
  set( COOL_PLATFORM_TARGET FREEBSD_TARGET )
  set( COOL_ASYNC_PLATFORM COOL_ASYNC_PLATFORM_GCD )
  set( COOL_TASK_RUNNER_IMPL GCD_DEQUE )
  
  # platform libraries to link
  set( COOL_NG_PLATFORM_LIBRARIES execinfo pthread dispatch )
  set( COOL_NG_COMPILER_OPTIONS -g -std=c++11 -fPIC -fbracket-depth=10000 )
  set( COOL_NG_ADDITIONAL_SYS_INCLUDES /usr/local/include )
  set( COOL_NG_ADDITIONAL_SYS_LIBDIRS /usr/local/lib )

elseif ( MSVC )

# --- Windows
  set( WINDOWS true )

  if ( MSVC_VERSION LESS_EQUAL 1800 )
    message ( FATAL_ERROR "Visual Studio versions earlier than Visual Studio 2015 are not supported. Please use Visual Studio 2015 or newer." )
  elseif( MSVC_VERSION LESS 1910 )
    set( COOL_NG_MSVC "14" )     # Visual Studio 2015 (14)
  elseif( MSVC_VERSION LESS 1920 )
    set( COOL_NG_MSVC "15" )     # Visual Studio 2017 (15.0, 15.3, 15.5, 15.6, 15.7, 15.8, 15.9)
  elseif( MSVC_VERSION LESS 1930 )
    set( COOL_NG_MSVC "16" )     # Visual Studi 2019 (16.0, 16.1, 16.2, 16.3)
  else()
    message( WARNING "Your Visual Studio appears newer than Visual Studio 2019. Cool.NG may or may not work with this Visual Studio." )
    set( COOL_NG_MSVC "16" )     # Visual Studi 2019 (16.0, 16.1, 16.2, 16.3)
  endif()

  # CMake incorrectly appends "/machine:X86" to all linker flags when static
  # libraries are used, resulting in invalid builds:
  # > fatal error LNK1112: module machine type 'x64' conflicts with target machine type 'X86'
  # (ref. https://public.kitware.com/Bug/view.php?id=15496)
  if (CMAKE_GENERATOR MATCHES "Win64")
    set( CMAKE_EXE_LINKER_FLAGS    "/machine:X64" )
    set( CMAKE_MODULE_LINKER_FLAGS "/machine:X64" )
    set( CMAKE_SHARED_LINKER_FLAGS "/machine:X64" )
  endif()

  set( COOL_PLATFORM_TARGET WINDOWS_TARGET )
  set( COOL_ASYNC_PLATFORM COOL_ASYNC_PLATFORM_WINCP )
  set( COOL_TASK_RUNNER_IMPL WIN_COMPLETION_PORT )

  add_definitions( -D_SCL_SECURE_NO_WARNINGS )  
  set( COOL_NG_COMPILER_OPTIONS
     /EHsc /bigobj /Zm750
     /wd4624 /wd4091 /wd4251 /wd4275
  )

  # platform libraries to link
  set( COOL_NG_PLATFORM_LIBRARIES DbgHelp.dll Ws2_32.dll )

else()

# --- unknown target
  message( FATAL_ERROR "Platform ${CMAKE_SYSTEM_NAME} is not supported" )

endif()
