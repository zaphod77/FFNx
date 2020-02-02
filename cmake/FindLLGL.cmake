include(FindPackageHandleStandardArgs)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Silence is golden
else()
    set(LLGL_SUFFIX "D")
endif()

if (NOT LLGL_FOUND)
    find_package(OpenGL REQUIRED)
		find_package(Vulkan REQUIRED)

	find_path(
		LLGL_INCLUDE_DIR
		LLGL
		PATH_SUFFIXES
		include
	)

	find_library(
		LLGL_LIBRARY
		LLGL${LLGL_SUFFIX}
		PATH_SUFFIXES
		lib/LLGL
	)

	add_library(LLGL::LLGL STATIC IMPORTED)

	set_target_properties(
		LLGL::LLGL
		PROPERTIES
		IMPORTED_LOCATION
		"${LLGL_LIBRARY}"
		INTERFACE_INCLUDE_DIRECTORIES
		"${LLGL_INCLUDE_DIR}"
	)

	# ---

	find_library(
		LLGL_OPENGL_LIBRARY
		LLGL_OpenGL${LLGL_SUFFIX}
		PATH_SUFFIXES
		lib/LLGL
	)

	add_library(LLGL::OPENGL STATIC IMPORTED)

	set_target_properties(
		LLGL::OPENGL
		PROPERTIES
		IMPORTED_LOCATION
		"${LLGL_OPENGL_LIBRARY}"
		INTERFACE_LINK_LIBRARIES
		"OpenGL::GL"
	)

	# ---

	find_library(
		LLGL_VULKAN_LIBRARY
		LLGL_Vulkan${LLGL_SUFFIX}
		PATH_SUFFIXES
		lib/LLGL
	)

	add_library(LLGL::VULKAN STATIC IMPORTED)

	set_target_properties(
		LLGL::VULKAN
		PROPERTIES
		IMPORTED_LOCATION
		"${LLGL_VULKAN_LIBRARY}"
		INTERFACE_LINK_LIBRARIES
		"Vulkan::Vulkan"
	)

	# ---

	find_library(
		LLGL_D3D11_LIBRARY
		LLGL_Direct3D11${LLGL_SUFFIX}
		PATH_SUFFIXES
		lib/LLGL
	)

	add_library(LLGL::D3D11 STATIC IMPORTED)

	set_target_properties(
		LLGL::D3D11
		PROPERTIES
		IMPORTED_LOCATION
		"${LLGL_D3D11_LIBRARY}"
		INTERFACE_LINK_LIBRARIES
		"dxgi;d3dcompiler;d3d11"
	)

	# ---

	find_library(
		LLGL_D3D12_LIBRARY
		LLGL_Direct3D12${LLGL_SUFFIX}
		PATH_SUFFIXES
		lib/LLGL
	)

	add_library(LLGL::D3D12 STATIC IMPORTED)

	set_target_properties(
		LLGL::D3D12
		PROPERTIES
		IMPORTED_LOCATION
		"${LLGL_D3D12_LIBRARY}"
		INTERFACE_LINK_LIBRARIES
		"dxgi;d3dcompiler;d3d12"
	)

	# ---

	find_package_handle_standard_args(LLGL DEFAULT_MSG
		# LLGL
		LLGL_LIBRARY
		LLGL_INCLUDE_DIR
		# LLGL OpenGL
		LLGL_OPENGL_LIBRARY
		# LLGL Vulkan
		LLGL_VULKAN_LIBRARY
		# LLGL Direct3D 11
		LLGL_D3D11_LIBRARY
		# LLGL Direct3D 12
		LLGL_D3D12_LIBRARY
  )
endif()
