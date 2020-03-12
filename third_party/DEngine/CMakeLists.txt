include(ExternalProject)

set(DENGINE_VERSION "2.4.0")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(DENGINE_RELEASE "Release")
else()
    set(DENGINE_RELEASE "Debug")
endif()

ExternalProject_Add(
	DiligentEngine
	URL					"https://github.com/julianxhokaxhiu/DiligentEngineCI/releases/download/${DENGINE_VERSION}/DiligentEngine-v${DENGINE_VERSION}_${DENGINE_RELEASE}.zip"
	INSTALL_DIR			"${CMAKE_BINARY_DIR}/vendor"
	CONFIGURE_COMMAND	""
	BUILD_COMMAND		""
	INSTALL_COMMAND		"${CMAKE_COMMAND}" -E copy_directory "<SOURCE_DIR>" "<INSTALL_DIR>"
)
