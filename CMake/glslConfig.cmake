if ( WIN32 )
  set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else( )
  set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/bin/glslangValidator")
endif( )

function( glslConfig )
	set(CFG_FILE "${PROJECT_BINARY_DIR}/config.config")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -c > ${CFG_FILE}
  list(APPEND GLSLCONFIG_FILE ${CFG_FILE})

	add_custom_target(
	  Glsl2Spv 
	  DEPENDS ${GLSLCONFIG_FILE}
	)

endfunction( )