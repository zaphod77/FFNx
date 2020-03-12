include(FindPackageHandleStandardArgs)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(DENGINE_RELEASE "Release")
else()
    set(DENGINE_RELEASE "Debug")
    set(DENGINE_SUFFIX "d")
endif()

if (NOT DENGINE_FOUND)
    find_package(OpenGL REQUIRED)
    find_package(Vulkan REQUIRED)

	find_library(
		DENGINE_LIBRARY
		DiligentCore
		PATH_SUFFIXES
		lib/DiligentCore/${DENGINE_RELEASE}
	)

	find_path(
		DENGINE_INCLUDE_DIR
		DiligentCore
		PATH_SUFFIXES
		include
	)

	add_library(DENGINE::DENGINE STATIC IMPORTED)

	set_target_properties(
		DENGINE::DENGINE
		PROPERTIES
		IMPORTED_LOCATION
		"${DENGINE_LIBRARY}"
		INTERFACE_INCLUDE_DIRECTORIES
		"${DENGINE_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES
        "OpenGL::GL;Vulkan::Vulkan;dxgi;d3d11;d3d12;d3dcompiler"
	)

    # ---

    find_library(
      GLEW_LIBRARY
      glew-static
      PATH_SUFFIXES
      lib/DiligentCore/${DENGINE_RELEASE}
    )

    add_library(DENGINE::GLEW STATIC IMPORTED)

    set_target_properties(
      DENGINE::GLEW
      PROPERTIES
      IMPORTED_LOCATION
      "${GLEW_LIBRARY}"
    )

    # ---

    find_library(
      GLSLANG_LIBRARY
      glslang${DENGINE_SUFFIX}
      PATH_SUFFIXES
      lib/DiligentCore/${DENGINE_RELEASE}
    )

    add_library(DENGINE::GLSLANG STATIC IMPORTED)

    set_target_properties(
      DENGINE::GLSLANG
      PROPERTIES
      IMPORTED_LOCATION
      "${GLSLANG_LIBRARY}"
    )

    # ---

    find_library(
      HLSL_LIBRARY
      HLSL${DENGINE_SUFFIX}
      PATH_SUFFIXES
      lib/DiligentCore/${DENGINE_RELEASE}
    )

    add_library(DENGINE::HLSL STATIC IMPORTED)

    set_target_properties(
      DENGINE::HLSL
      PROPERTIES
      IMPORTED_LOCATION
      "${HLSL_LIBRARY}"
    )

    # ---

    find_library(
      OGLCOMPILER_LIBRARY
      OGLCompiler${DENGINE_SUFFIX}
      PATH_SUFFIXES
      lib/DiligentCore/${DENGINE_RELEASE}
    )

    add_library(DENGINE::OGLCOMPILER STATIC IMPORTED)

    set_target_properties(
      DENGINE::OGLCOMPILER
      PROPERTIES
      IMPORTED_LOCATION
      "${OGLCOMPILER_LIBRARY}"
    )

    # ---

    find_library(
      OSDEPENDENT_LIBRARY
      OSDependent${DENGINE_SUFFIX}
      PATH_SUFFIXES
      lib/DiligentCore/${DENGINE_RELEASE}
    )

    add_library(DENGINE::OSDEPENDENT STATIC IMPORTED)

    set_target_properties(
      DENGINE::OSDEPENDENT
      PROPERTIES
      IMPORTED_LOCATION
      "${OSDEPENDENT_LIBRARY}"
    )

    # ---

    find_library(
      SPIRVCROSSCORE_LIBRARY
      spirv-cross-core${DENGINE_SUFFIX}
      PATH_SUFFIXES
      lib/DiligentCore/${DENGINE_RELEASE}
    )

    add_library(DENGINE::SPIRVCROSSCORE STATIC IMPORTED)

    set_target_properties(
      DENGINE::SPIRVCROSSCORE
      PROPERTIES
      IMPORTED_LOCATION
      "${SPIRVCROSSCORE_LIBRARY}"
    )

    # ---

    find_library(
      SPIRVTOOLSOPT_LIBRARY
      SPIRV-Tools-opt
      PATH_SUFFIXES
      lib/DiligentCore/${DENGINE_RELEASE}
    )

    add_library(DENGINE::SPIRVTOOLSOPT STATIC IMPORTED)

    set_target_properties(
      DENGINE::SPIRVTOOLSOPT
      PROPERTIES
      IMPORTED_LOCATION
      "${SPIRVTOOLSOPT_LIBRARY}"
    )

    # ---

    find_library(
      SPIRVTOOLS_LIBRARY
      SPIRV-Tools
      PATH_SUFFIXES
      lib/DiligentCore/${DENGINE_RELEASE}
    )

    add_library(DENGINE::SPIRVTOOLS STATIC IMPORTED)

    set_target_properties(
      DENGINE::SPIRVTOOLS
      PROPERTIES
      IMPORTED_LOCATION
      "${SPIRVTOOLS_LIBRARY}"
    )

    # ---

    find_library(
      SPIRV_LIBRARY
      SPIRV${DENGINE_SUFFIX}
      PATH_SUFFIXES
      lib/DiligentCore/${DENGINE_RELEASE}
    )

    add_library(DENGINE::SPIRV STATIC IMPORTED)

    set_target_properties(
      DENGINE::SPIRV
      PROPERTIES
      IMPORTED_LOCATION
      "${SPIRV_LIBRARY}"
    )

    # ---

    find_library(
      DTOOLS_LIBRARY
      DiligentTools
      PATH_SUFFIXES
      lib/DiligentTools/${DENGINE_RELEASE}
    )

    find_path(
      DTOOLS_INCLUDE_DIR
      DiligentTools
      PATH_SUFFIXES
      include
    )

    add_library(DENGINE::TOOLS STATIC IMPORTED)

    set_target_properties(
      DENGINE::TOOLS
      PROPERTIES
      IMPORTED_LOCATION
      "${DTOOLS_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES
	  "${DTOOLS_INCLUDE_DIR}"
    )

    # ---

    find_library(
      LIBPNG_LIBRARY
      LibPng
      PATH_SUFFIXES
      lib/DiligentTools/${DENGINE_RELEASE}
    )

    add_library(DENGINE::LIBPNG STATIC IMPORTED)

    set_target_properties(
      DENGINE::LIBPNG
      PROPERTIES
      IMPORTED_LOCATION
      "${LIBPNG_LIBRARY}"
    )

    # ---

    find_library(
      LIBJPEG_LIBRARY
      LibJpeg
      PATH_SUFFIXES
      lib/DiligentTools/${DENGINE_RELEASE}
    )

    add_library(DENGINE::LIBJPEG STATIC IMPORTED)

    set_target_properties(
      DENGINE::LIBJPEG
      PROPERTIES
      IMPORTED_LOCATION
      "${LIBJPEG_LIBRARY}"
    )

    # ---

    find_library(
      LIBTIFF_LIBRARY
      LibTiff
      PATH_SUFFIXES
      lib/DiligentTools/${DENGINE_RELEASE}
    )

    add_library(DENGINE::LIBTIFF STATIC IMPORTED)

    set_target_properties(
      DENGINE::LIBTIFF
      PROPERTIES
      IMPORTED_LOCATION
      "${LIBTIFF_LIBRARY}"
    )

    # ---

    find_library(
      ZLIB_LIBRARY
      ZLib
      PATH_SUFFIXES
      lib/DiligentTools/${DENGINE_RELEASE}
    )

    add_library(DENGINE::ZLIB STATIC IMPORTED)

    set_target_properties(
      DENGINE::ZLIB
      PROPERTIES
      IMPORTED_LOCATION
      "${ZLIB_LIBRARY}"
    )

    # ---

	find_package_handle_standard_args(DENGINE DEFAULT_MSG
        # DiligentEngine
        DENGINE_LIBRARY
        DENGINE_INCLUDE_DIR
        GLEW_LIBRARY
        GLSLANG_LIBRARY
        HLSL_LIBRARY
        OGLCOMPILER_LIBRARY
        OSDEPENDENT_LIBRARY
        SPIRVCROSSCORE_LIBRARY
        SPIRVTOOLSOPT_LIBRARY
        SPIRVTOOLS_LIBRARY
        SPIRV_LIBRARY
        # DiligentTools
        DTOOLS_LIBRARY
        DTOOLS_INCLUDE_DIR
        LIBJPEG_LIBRARY
        LIBPNG_LIBRARY
        LIBTIFF_LIBRARY
        ZLIB_LIBRARY
    )
endif()
