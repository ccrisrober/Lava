#source_group(\\ FILES CMakeLists.txt)
set( LAVAENGINE_PUBLIC_HEADERS
	Mathematics/Frustum.h
	Mathematics/Mathf.h

	Scenegraph/Node.h
	Scenegraph/Group.h
	Scenegraph/Light.h
	Scenegraph/Switch.h
	Scenegraph/Camera.h
	Scenegraph/Scene.h

	Visitors/Visitor.h
	Visitors/FindNodes.h
	Visitors/FetchLights.h
	Visitors/DumpVisitor.h
	Visitors/LambdaVisitor.h
	Visitors/PrintNameVisitor.h
	Visitors/ChildrenCounterVisitor.h
	Visitors/FetchCameras.h

	Utils/Easing.h
	Utils/Macros.h
	Utils/Layer.h
)

set( LAVAENGINE_SOURCES

	Mathematics/Frustum.cpp
	Mathematics/Mathf.cpp

	Scenegraph/Node.cpp
	Scenegraph/Group.cpp
	Scenegraph/Light.cpp
	Scenegraph/Switch.cpp
	Scenegraph/Camera.cpp
	Scenegraph/Scene.cpp

	Visitors/Visitor.cpp
	Visitors/FindNodes.cpp
	Visitors/FetchLights.cpp
	Visitors/DumpVisitor.cpp
	Visitors/LambdaVisitor.cpp
	Visitors/PrintNameVisitor.cpp
	Visitors/ChildrenCounterVisitor.cpp
	Visitors/FetchCameras.cpp

	Utils/Easing.cpp
	Utils/Layer.cpp
)

set( LAVAENGINE_HEADERS
)

set( LAVAENGINE_MOC_HEADERS
)

if( LAVAENGINE_WITH_COMPONENTS )
	list( APPEND LAVAENGINE_PUBLIC_HEADERS 
		Components/Component.h
		#Components/LambdaComponent.h
	)

	list( APPEND LAVAENGINE_SOURCES
		Components/Component.cpp
		#Components/LambdaComponent.cpp
	)
endif( )

set( LAVAENGINE_LINK_LIBRARIES
	lava
	lavaUtils
	## ${GLM_LIBRARIES}		Required by lavaUtils
	## ${ASSIMP_LIBRARIES}	Required by lavaUtils
)
set( LAVAENGINE_INCLUDE_NAME lavaEngine )
set( LAVAENGINE_NAMESPACE lavaEngine )
common_library( lavaEngine )
