include(ExternalProject)

set(LLGL_VERSION "0.0.3")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(LLGL_RELEASE "Release")
else()
    set(LLGL_RELEASE "Debug")
endif()

ExternalProject_Add(
	LLGL
	URL                 "https://github.com/julianxhokaxhiu/LLGLCI/releases/download/${LLGL_VERSION}/LLGL-v${LLGL_VERSION}.0_${LLGL_RELEASE}.zip"
	INSTALL_DIR			"${CMAKE_BINARY_DIR}/vendor"
	CONFIGURE_COMMAND	""
	BUILD_COMMAND		""
	INSTALL_COMMAND		"${CMAKE_COMMAND}" -E copy_directory "<SOURCE_DIR>" "<INSTALL_DIR>"
)
