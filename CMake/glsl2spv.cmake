if ( WIN32 )
	set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else( )
	set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/bin/glslangValidator")
endif( )

file(GLOB_RECURSE GLSL_SOURCE_FILES
	"${CMAKE_SOURCE_DIR}/resources/shaders/*.vert"
	"${CMAKE_SOURCE_DIR}/resources/shaders/*.frag"
	"${CMAKE_SOURCE_DIR}/resources/shaders/*.comp"
	"${CMAKE_SOURCE_DIR}/resources/shaders/*.geom"
	"${CMAKE_SOURCE_DIR}/resources/shaders/*.tesc"
	"${CMAKE_SOURCE_DIR}/resources/shaders/*.tese"
)
function( glsl2spv )
	foreach(GLSL ${GLSL_SOURCE_FILES})
		get_filename_component(FILE_NAME ${GLSL} NAME_WE)
		get_filename_component(FILE_EXT_AUX ${GLSL} EXT)
		STRING(REPLACE "." "" FILE_EXT ${FILE_EXT_AUX})
		set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}_${FILE_EXT}.spv")
		add_custom_command(
			OUTPUT ${SPIRV}
			COMMAND ${CMAKE_COMMAND} -E make_directory 
				"${PROJECT_BINARY_DIR}/shaders/"
			COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
			DEPENDS ${GLSL})
		list(APPEND SPIRV_BINARY_FILES ${SPIRV})
	endforeach(GLSL)
	
	add_custom_target(
		Glsl2Spv 
		DEPENDS ${SPIRV_BINARY_FILES}
	)

endfunction( )