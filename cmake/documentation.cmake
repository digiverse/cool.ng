
# ### ##################################################
# ###
# ### Doxygen documentation
# ###
# ### ##################################################

macro( cool_ng_api_doc DocHome Target OutputDir)

  find_package(Doxygen OPTIONAL_COMPONENTS dot)
  if( NOT DOXYGEN_FOUND )
    message(WARNING "Doxygen package was not found, will not generate the Cool.NG API documentation")
  else()

    if( ${CMAKE_VERSION} VERSION_LESS "3.9.0" )
      message ( WARNING "Your cmake is version ${CMAKE_VERSION}. This build system requires version 3.9 or later to build the Doxygen documentation.")
    else()

      if( ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0" )
	set( DO_USE_TIMESTAMP USE_STAMP_FILE)
      endif()
      if( ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0" )
        set( DO_USE_ALL ALL)
      endif()

      set( DOXYGEN_PROJECT_NAME "Cool.NG API Reference")
      set( DOXYGEN_EXTRACT_ALL NO )
      set( DOXYGEN_GENERATE_HTML YES )
      set( DOXYGEN_GENERATE_MAN NO )
      set( DOXYGEN_OUTPUT_DIRECTORY ${OutputDir} )
      set( DOXYGEN_PREDEFINED GENERATE_DOXYGEN_DOC )
    
      doxygen_add_docs(
	${Target}
	${ARGN}
	${DO_USE_ALL}
	${DO_USE_TIMESTAMP}
      )

    endif()

  endif()
  
endmacro()
