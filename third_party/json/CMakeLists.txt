include(ExternalProject)

ExternalProject_Add(
    json
    URL                 "https://github.com/nlohmann/json/releases/download/v3.7.3/include.zip"
    URL_HASH            "SHA256=87b5884741427220d3a33df1363ae0e8b898099fbc59f1c451113f6732891014"
    INSTALL_DIR			"${CMAKE_BINARY_DIR}/vendor"
	CONFIGURE_COMMAND	""
	BUILD_COMMAND		""
	INSTALL_COMMAND		"${CMAKE_COMMAND}" -E copy_directory "<SOURCE_DIR>/single_include" "<INSTALL_DIR>/include"
)
