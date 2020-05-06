include(ExternalProject)

ExternalProject_Add(
    SisterRay
    URL                 "https://github.com/FFT-Hackers/SisterRay/archive/master.zip"
	INSTALL_DIR			"${CMAKE_BINARY_DIR}/vendor"
    CONFIGURE_COMMAND	""
	BUILD_COMMAND		""
	INSTALL_COMMAND		"${CMAKE_COMMAND}" -E copy_directory "<SOURCE_DIR>/include" "<INSTALL_DIR>/include"
)
