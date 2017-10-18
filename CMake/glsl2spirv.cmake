if ( WIN32 )
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else( )
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/bin/glslangValidator")
endif( )

function(compileSPV)
    set(SPV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
    file(MAKE_DIRECTORY ${SPV_DIR}/spvs)
    
    file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${SPV_DIR}/resources/shaders/*.vert"
        "${SPV_DIR}/resources/shaders/*.frag"
        "${SPV_DIR}/resources/shaders/*.comp"
        "${SPV_DIR}/resources/shaders/*.geom"
        "${SPV_DIR}/resources/shaders/*.tesc"
        "${SPV_DIR}/resources/shaders/*.tese"
    )
    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME_WE)
        get_filename_component(FILE_EXT_AUX ${GLSL} EXT)
        STRING(REPLACE "." "" FILE_EXT ${FILE_EXT_AUX})
        set(SPIRV_FILE "${SPV_DIR}/spvs/${FILE_NAME}_${FILE_EXT}.spv")
        execute_process(
            COMMAND ${EXECUTE_COMMAND} ${GLSL_VALIDATOR} 
                -V ${GLSL} -o ${SPIRV_FILE}
        )
    endforeach(GLSL)
endfunction(compileSPV)

compileSPV( )