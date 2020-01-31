find_package(GLEW REQUIRED)
find_package(ZLib REQUIRED)
find_package(PNG REQUIRED)
find_package(CONFUSE REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(VGMSTREAM REQUIRED)

# Include all the source code files
file(GLOB_RECURSE source_files "*.h" "*.c")

add_library(FFNx SHARED ${source_files})
target_include_directories(FFNx PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(FFNx
	dbghelp
	GLEW::GLEW
	PNG::PNG
	CONFUSE::CONFUSE
	FFMPEG::AVUtil
	FFMPEG::SWResample
	FFMPEG::AVCodec
	FFMPEG::AVFormat
	FFMPEG::SWScale
	VGMSTREAM::VGMSTREAM
)
target_compile_options(FFNx
	PRIVATE /D_CRT_SECURE_NO_WARNINGS
)
target_link_options(FFNx PRIVATE /PDBALTPATH:FFNx.pdb)
if(_DLL_VERSION)
	target_compile_options(FFNx PRIVATE /DVERSION="${_DLL_VERSION}")
endif()

install(TARGETS FFNx RUNTIME DESTINATION bin)
install(FILES $<TARGET_PDB_FILE:FFNx> DESTINATION bin OPTIONAL)
install(FILES ../misc/FFNx.cfg DESTINATION bin OPTIONAL)
install(FILES ../misc/FFNx.reg DESTINATION bin OPTIONAL)
