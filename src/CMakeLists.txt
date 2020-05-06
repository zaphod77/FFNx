find_package(BGFX REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(VGMSTREAM REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(RELEASE_TYPE "RELEASE")
else()
    set(RELEASE_TYPE "DEBUG")
endif()

set(RELEASE_NAME "FFNx")

# Include all the source code files
file(GLOB_RECURSE source_files "*.h" "*.hpp" "*.cpp")

add_library(${RELEASE_NAME} SHARED ${source_files})
target_include_directories(${RELEASE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${RELEASE_NAME}
	dbghelp
    shlwapi
    psapi
    BGFX::BGFX
    BGFX::BIMG
    BGFX::BX
	FFMPEG::AVUtil
	FFMPEG::SWResample
	FFMPEG::AVCodec
	FFMPEG::AVFormat
	FFMPEG::SWScale
    VGMSTREAM::VGMSTREAM
)
target_compile_options(${RELEASE_NAME}
	PRIVATE /D_CRT_SECURE_NO_WARNINGS
	PRIVATE /D${RELEASE_TYPE}
    PRIVATE /D_DLLOUTNAME="${RELEASE_NAME}"
)
target_link_options(${RELEASE_NAME} PRIVATE /PDBALTPATH:${RELEASE_NAME}.pdb)
if(_DLL_VERSION)
	target_compile_options(${RELEASE_NAME} PRIVATE /D_DLLVERSION="${_DLL_VERSION}")
endif()

# SHADER COMPILATION
add_custom_command(
    OUTPUT ${RELEASE_NAME}_shaders
    # ensure bin directory exists
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
    # fragment shader
    COMMAND ${CMAKE_INSTALL_PREFIX}/vendor/bin/shaderc -i ${CMAKE_INSTALL_PREFIX}/vendor/include -f ${CMAKE_SOURCE_DIR}/misc/${RELEASE_NAME}.frag -o ${CMAKE_BINARY_DIR}/bin/FFNx.frag.bin --type f --varyingdef ${CMAKE_SOURCE_DIR}/misc/${RELEASE_NAME}.varying.def.sc #--platform linux --profile spirv
    # vertex shader
    COMMAND ${CMAKE_INSTALL_PREFIX}/vendor/bin/shaderc -i ${CMAKE_INSTALL_PREFIX}/vendor/include -f ${CMAKE_SOURCE_DIR}/misc/${RELEASE_NAME}.vert -o ${CMAKE_BINARY_DIR}/bin/FFNx.vert.bin --type v --varyingdef ${CMAKE_SOURCE_DIR}/misc/${RELEASE_NAME}.varying.def.sc #--platform linux --profile spirv
)

add_custom_target(${RELEASE_NAME}_build_shaders
    DEPENDS ${RELEASE_NAME}_shaders
)

add_dependencies(${RELEASE_NAME} ${RELEASE_NAME}_build_shaders)

# INSTALL

install(TARGETS ${RELEASE_NAME} RUNTIME DESTINATION bin)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	install(FILES $<TARGET_PDB_FILE:${RELEASE_NAME}> DESTINATION bin OPTIONAL)
endif()

install(FILES ../misc/${RELEASE_NAME}.reg DESTINATION bin OPTIONAL)
install(FILES ${CMAKE_BINARY_DIR}/bin/FFNx.frag.bin DESTINATION bin OPTIONAL)
install(FILES ${CMAKE_BINARY_DIR}/bin/FFNx.vert.bin DESTINATION bin OPTIONAL)
