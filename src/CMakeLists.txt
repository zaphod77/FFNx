find_package(LLGL REQUIRED)
find_package(ZLib REQUIRED)
find_package(PNG REQUIRED)
find_package(CONFUSE REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(VGMSTREAM REQUIRED)

set(RELEASE_NAME "FFNx")

# Include all the source code files
file(GLOB_RECURSE source_files "*.h" "*.cpp")

add_library(${RELEASE_NAME} SHARED ${source_files})
target_include_directories(${RELEASE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${RELEASE_NAME}
	dbghelp
	LLGL::LLGL
	LLGL::OPENGL
	LLGL::VULKAN
	LLGL::D3D11
	LLGL::D3D12
	PNG::PNG
	CONFUSE::CONFUSE
	FFMPEG::AVUtil
	FFMPEG::SWResample
	FFMPEG::AVCodec
	FFMPEG::AVFormat
	FFMPEG::SWScale
	VGMSTREAM::VGMSTREAM
)
target_compile_options(${RELEASE_NAME}
	PRIVATE /D_CRT_SECURE_NO_WARNINGS
)
target_link_options(${RELEASE_NAME} PRIVATE /PDBALTPATH:${RELEASE_NAME}.pdb)
if(_DLL_VERSION)
	target_compile_options(${RELEASE_NAME} PRIVATE /DVERSION="${_DLL_VERSION}")
endif()

# SHADER COMPILATION
add_custom_command(
    OUTPUT ${RELEASE_NAME}_shaders
    # ensure bin directory exists
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
    # fragment shader
    COMMAND $ENV{VULKAN_SDK}/Bin32/glslangValidator -S frag -e FSMain -o ${CMAKE_BINARY_DIR}/bin/${RELEASE_NAME}.frag.spv -V -D ${CMAKE_SOURCE_DIR}/misc/${RELEASE_NAME}.hlsl
    # vertex shader
    COMMAND $ENV{VULKAN_SDK}/Bin32/glslangValidator -S vert -e VSMain -o ${CMAKE_BINARY_DIR}/bin/${RELEASE_NAME}.vert.spv -V -D ${CMAKE_SOURCE_DIR}/misc/${RELEASE_NAME}.hlsl
)

add_custom_target(${RELEASE_NAME}_build_shaders
    DEPENDS ${RELEASE_NAME}_shaders
)

add_dependencies(${RELEASE_NAME} ${RELEASE_NAME}_build_shaders)

# INSTALL
install(TARGETS ${RELEASE_NAME} RUNTIME DESTINATION bin)
install(FILES $<TARGET_PDB_FILE:${RELEASE_NAME}> DESTINATION bin OPTIONAL)
install(FILES ../misc/${RELEASE_NAME}.cfg DESTINATION bin OPTIONAL)
install(FILES ../misc/${RELEASE_NAME}.reg DESTINATION bin OPTIONAL)
# Shaders
install(DIRECTORY DESTINATION bin/shaders)
## OpenGL
install(FILES ../misc/${RELEASE_NAME}.vert DESTINATION bin/shaders OPTIONAL)
install(FILES ../misc/${RELEASE_NAME}.frag DESTINATION bin/shaders OPTIONAL)
## Direct3D 11/12
install(FILES ../misc/${RELEASE_NAME}.hlsl DESTINATION bin/shaders OPTIONAL)
## Vulkan
install(FILES ${CMAKE_BINARY_DIR}/bin/${RELEASE_NAME}.vert.spv DESTINATION bin/shaders OPTIONAL)
install(FILES ${CMAKE_BINARY_DIR}/bin/${RELEASE_NAME}.frag.spv DESTINATION bin/shaders OPTIONAL)
