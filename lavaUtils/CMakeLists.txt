#source_group(\\ FILES CMakeLists.txt)
# if( ASSIMP_FOUND )
set( LAVAUTILS_PUBLIC_HEADERS
	Mesh.h
	Material.h
	ModelImporter.h
	Geometry.h
	CustomMaterial.h
)

set( LAVAUTILS_SOURCES
	Mesh.cpp
	Material.cpp
	ModelImporter.cpp
	Geometry.cpp
	CustomMaterial.cpp
)

set( LAVAUTILS_HEADERS
)

set( LAVAUTILS_MOC_HEADERS
)

set( LAVAUTILS_LINK_LIBRARIES
	lava
	${GLM_LIBRARIES}
	${ASSIMP_LIBRARIES}
)
set( LAVAUTILS_INCLUDE_NAME lavaUtils )
set( LAVAUTILS_NAMESPACE lavaUtils )
common_library( lavaUtils )
