include_directories( 
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -DNOMINMAX
)

source_group(\\ FILES CMakeLists.txt)

set( POMPEIIUTILS_PUBLIC_HEADERS
	Mesh.h
	Material.h
	ModelImporter.h
	Geometry.h
	CustomMaterial.h
	ThreadPool.h

	CustomFramebuffer.h
	CustomPingPong.h

	Wrappers.h

	glmConfig.h

	States.h

	Compiler.h
)

set( POMPEIIUTILS_SOURCES
	Mesh.cpp
	Material.cpp
	ModelImporter.cpp
	Geometry.cpp
	CustomMaterial.cpp
	ThreadPool.cpp

	CustomFramebuffer.cpp

	Wrappers.cpp

	States.cpp

	Compiler.cpp
)

set( POMPEIIUTILS_LINK_LIBRARIES
	pompeii
	${GLM_LIBRARIES}
	${ASSIMP_LIBRARIES}
	
	glslang
	OGLCompiler
	OSDependent
	SPIRV
)

set( POMPEIIUTILS_PUBLIC_INCLUDE_DIRECTORIES
	"${CMAKE_SOURCE_DIR}/vendor"
)

set( POMPEIIUTILS_INCLUDE_NAME pompeiiUtils )
set( POMPEIIUTILS_NAMESPACE pompeiiUtils )

common_library( pompeiiUtils )