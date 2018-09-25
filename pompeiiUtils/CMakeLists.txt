#source_group(\\ FILES CMakeLists.txt)
# if( ASSIMP_FOUND )

#file(GLOB BASE_SRC "*.cpp" "../thirdparty/imgui/imgui.cpp" "../thirdparty/imgui/imgui_draw.cpp")

set( POMPEIIUTILS_PUBLIC_HEADERS
	Mesh.h
	Material.h
	ModelImporter.h
	Geometry.h
	CustomMaterial.h
	ThreadPool.h
	###Glsl2SPV.h
	CustomFramebuffer.h
	CustomPingPong.h

	Wrappers.h

	##../thirdparty/imgui/imgui.h
	##VulkanUI.h

	glmConfig.h

	States.h

	#Compiler.h
)

set( POMPEIIUTILS_SOURCES
	Mesh.cpp
	Material.cpp
	ModelImporter.cpp
	Geometry.cpp
	CustomMaterial.cpp
	ThreadPool.cpp
	###Glsl2SPV.cpp
	CustomFramebuffer.cpp

	Wrappers.cpp

	##../thirdparty/imgui/imgui.cpp
	##../thirdparty/imgui/imgui_draw.cpp

	##VulkanUI.cpp

	States.cpp

	#Compiler.cpp
)

set( POMPEIIUTILS_HEADERS
)

set( POMPEIIUTILS_MOC_HEADERS
)

set( POMPEIIUTILS_LINK_LIBRARIES
	pompeii
	${GLM_LIBRARIES}
	${ASSIMP_LIBRARIES}
	
##	glslang
##	OGLCompiler
##	OSDependent
##	#HLSL
##	SPIRV
)
##set( POMPEIIUTILS_PUBLIC_INCLUDE_DIRECTORIES
##	"${CMAKE_SOURCE_DIR}/vendor"
##)
set( POMPEIIUTILS_INCLUDE_NAME pompeiiUtils )
set( POMPEIIUTILS_NAMESPACE pompeiiUtils )
common_library( pompeiiUtils )
