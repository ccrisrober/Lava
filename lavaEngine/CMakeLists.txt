#source_group(\\ FILES CMakeLists.txt)
set( LAVAENGINE_PUBLIC_HEADERS
	#Components/Component.h
	#Components/LambdaComponent.h

	Mathematics/Frustum.h

	Scenegraph/Node.h
	Scenegraph/Group.h
	Scenegraph/Light.h
	Scenegraph/Switch.h
	Scenegraph/Camera.h

	Visitors/Visitor.h
	Visitors/FindNodes.h
	Visitors/FetchLights.h
	Visitors/DumpVisitor.h
	Visitors/LambdaVisitor.h
	Visitors/PrintNameVisitor.h
	Visitors/ChildrenCounterVisitor.h
)

set( LAVAENGINE_SOURCES
	#Components/Component.cpp
	#Components/LambdaComponent.cpp

	Mathematics/Frustum.cpp

	Scenegraph/Node.cpp
	Scenegraph/Group.cpp
	Scenegraph/Light.cpp
	Scenegraph/Switch.cpp
	Scenegraph/Camera.cpp

	Visitors/Visitor.cpp
	Visitors/FindNodes.cpp
	Visitors/FetchLights.cpp
	Visitors/DumpVisitor.cpp
	Visitors/LambdaVisitor.cpp
	Visitors/PrintNameVisitor.cpp
	Visitors/ChildrenCounterVisitor.cpp
)

set( LAVAENGINE_HEADERS
)

set( LAVAENGINE_MOC_HEADERS
)

set( LAVAENGINE_LINK_LIBRARIES
	lava
	lavaUtils
	## ${GLM_LIBRARIES}		Required by lavaUtils
	## ${ASSIMP_LIBRARIES}	Required by lavaUtils
)
set( LAVAENGINE_INCLUDE_NAME lavaEngine )
set( LAVAENGINE_NAMESPACE lavaEngine )
common_library( lavaEngine )
