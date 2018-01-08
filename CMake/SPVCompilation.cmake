if ( WIN32 )
  set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else( )
  set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/bin/glslangValidator")
endif( )

MESSAGE("${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/..resources/shaders/*.vert"
  "${CMAKE_CURRENT_SOURCE_DIR}/..resources/shaders/*.frag"
  "${CMAKE_CURRENT_SOURCE_DIR}/..resources/shaders/*.comp"
  "${CMAKE_CURRENT_SOURCE_DIR}/..resources/shaders/*.geom"
  "${CMAKE_CURRENT_SOURCE_DIR}/..resources/shaders/*.tesc"
  "${CMAKE_CURRENT_SOURCE_DIR}/..resources/shaders/*.tese"
)
MESSAGE( "${GLSL_SOURCE_FILES}")