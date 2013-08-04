# "sources" - unexpanded cmake variable holding all the source files
# "target_name" - the name of the a special target used to build the PCH for GCC
# "header_name" - the name of the PCH header, without the extension; "stdafx" or something similar;
#                  note that the source file compiling the header needs to have the same name 
# "ignored_files" - files to not process for precompiled header (optional)
macro( precompiled_header sources target_name header_name ignored_files)

	# MSVC precompiled headers cmake code ## @@ TODO
	if (MSVC)
		foreach( src_file ${sources} )
			if( ${src_file} MATCHES ".*cpp$" )
				set(position -1)
				foreach(file ${ignored_files})
					STRING(COMPARE EQUAL ${file} ${src_file} result)
					if(${result})
						set(position 0)
					endif()
				endforeach()
				if(position EQUAL -1)
					set_source_files_properties( ${src_file} PROPERTIES COMPILE_FLAGS "/Fp${header_name}_cpp.pch /Yu${header_name}.h" )
				endif()
			endif()
		endforeach()

		set_source_files_properties( ${header_name}.cpp PROPERTIES COMPILE_FLAGS "/Fp${header_name}_cpp.pch /Yc${header_name}.h" )

	# GCC precompiled headers cmake code
	else()
		set(compiler ${CMAKE_CXX_COMPILER})
		string( TOUPPER "CMAKE_CXX_FLAGS" flags_for_build_name )
		set( compile_flags ${${flags_for_build_name}} )

		# Get the list of all build-independent preprocessor definitions
		get_directory_property( defines_global COMPILE_DEFINITIONS )
		list( APPEND defines ${defines_global} )

		# Get the list of all build-dependent preprocessor definitions
		string( TOUPPER "COMPILE_DEFINITIONS" defines_for_build_name )
		get_directory_property( defines_build ${defines_for_build_name} )
		list( APPEND defines ${defines_build} )

		# Add the "-D" prefix to all of them
		foreach( item ${defines} )

			if( "${all_define_flags}" MATCHES "${item}" )
			else()
				list( APPEND all_define_flags "-D${item}" )
			endif()

		endforeach()

		# Get the list of all includes for this module
		get_directory_property( includes_global INCLUDE_DIRECTORIES)
		list( APPEND includes ${includes_global} )
		list( APPEND includes "-I${CMAKE_CURRENT_BINARY_DIR}" )

		# Add the "-I" prefix to all of them
		foreach( item ${includes} )
			list( APPEND all_define_flags "-I${item}" )
		endforeach()

		add_definitions(-Winvalid-pch)
		list(APPEND all_define_flags "-gdwarf-2 ")
		list(APPEND all_define_flags "-fPIC ")
		if(PROC_X86)
			list(APPEND all_define_flags "-msse2 ")
		endif()
		if(PROC_X86_64)
			list(APPEND all_define_flags "-msse3 ")
		endif()
		list(APPEND all_define_flags "-Wno-deprecated ")
		if(IOS)
			list(APPEND all_define_flags "-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator6.1.sdk -mios-simulator-version-min=5.0")
			if(PROC_X86)
				list(APPEND all_define_flags "-mios-simulator-version-min=5.0 ")
			else()
				list(APPEND all_define_flags "-miphoneos-version-min=5.0 ")
			endif()
		elseif(APPLE)
			list(APPEND all_define_flags "-mmacosx-version-min=10.7 ")
		endif()
		if(BRTBUILD_DEBUG)
			list(APPEND all_define_flags "-O0")
		else()
			list(APPEND all_define_flags "-O2")
			list(APPEND all_define_flags "-DNDEBUG")	
		endif()
		list(APPEND all_define_flags "-fvisibility=hidden")

		list( APPEND compile_flags ${all_define_flags} ) 

		# Prepare the compile flags var for passing to GCC
		separate_arguments( compile_flags )

		# This screws up pch's in clang
		LIST(REMOVE_ITEM compile_flags "-save-temps")
		LIST(REMOVE_ITEM all_define_flags "-save-temps")

		# need to make the pch dir
		file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${header_name}.h.gch)

		if(DISABLE_PCH)
			add_custom_target(${target_name} ALL)
		else()
			add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${header_name}.h.gch/cpp.gch 
				COMMAND ${compiler} ${compile_flags} -c ${CMAKE_CURRENT_SOURCE_DIR}/${header_name}.h -o ${CMAKE_CURRENT_BINARY_DIR}/${header_name}.h.gch/cpp.gch
				DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${header_name}.h
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				VERBATIM )

			add_custom_target(${target_name} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${header_name}.h.gch/cpp.gch)

			if(CLANG)
				foreach( src_file ${sources} )
					if(${src_file} MATCHES ".*cpp$")
						set(position -1)
						foreach(file ${ignored_files})
							STRING(COMPARE EQUAL ${file} ${src_file} result)
							if(${result})
								set(position 0)
							endif()
						endforeach()
						if(position EQUAL -1)
							set_source_files_properties( ${src_file} PROPERTIES COMPILE_FLAGS "-include-pch ${CMAKE_CURRENT_BINARY_DIR}/${header_name}.h.gch/cpp.gch" )
						endif()
					endif()
				endforeach()
			endif(CLANG)
		endif(DISABLE_PCH)
	endif() 
endmacro()
