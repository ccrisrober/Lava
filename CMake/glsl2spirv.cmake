if ( WIN32 )
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else( )
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/bin/glslangValidator")
endif( )


function(compileSPV)
    file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/spvs)
    file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${PROJECT_SOURCE_DIR}/resources/shaders/*.vert"
        "${PROJECT_SOURCE_DIR}/resources/shaders/*.frag"
        "${PROJECT_SOURCE_DIR}/resources/shaders/*.comp"
        "${PROJECT_SOURCE_DIR}/resources/shaders/*.geom"
        "${PROJECT_SOURCE_DIR}/resources/shaders/*.tesc"
        "${PROJECT_SOURCE_DIR}/resources/shaders/*.tese"
    )
    foreach(GLSL ${GLSL_SOURCE_FILES})
        #MESSAGE("HOLA")
        get_filename_component(FILE_NAME ${GLSL} NAME_WE)
        get_filename_component(FILE_EXT_AUX ${GLSL} EXT)
        STRING(REPLACE "." "" FILE_EXT ${FILE_EXT_AUX})
        set(SPIRV_FILE "${PROJECT_SOURCE_DIR}/spvs/${FILE_NAME}_${FILE_EXT}.spv")
        ##add_custom_command(
        ##    OUTPUT ${SPIRV_FILE}
        ##    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/resources/"
        ##    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV_FILE}
        ##    DEPENDS ${GLSL})
        ##list(APPEND SPIRV_BINARY_FILES ${SPIRV_FILE})
        execute_process(
            COMMAND ${EXECUTE_COMMAND} ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV_FILE}
        )

        #add_custom_command(
        #    COMMAND ${GLSL_VALIDATOR}
        #    ARGS
        #    -V ${GLSL}
        #    -o ${SPIRV_FILE}
        #    DEPENDS ${GLSL}
        #    OUTPUT ${SPIRV_FILE}
        #    MAIN_DEPENDENCY ${SPIRV_FILE}
        #)
    endforeach(GLSL)
endfunction(compileSPV)