#source_group(\\ FILES CMakeLists.txt)
set( LAVAENGINE_PUBLIC_HEADERS
	glm_config.h
	
	Exceptions/Exception.h
	Exceptions/FileNotFoundException.h
	Exceptions/HasParentException.h
	Exceptions/RuntimeException.h

	#Mathematics/Color.h
	Mathematics/Frustum.h
	Mathematics/Mathf.h
	Mathematics/Ray.h
	Mathematics/Spherical.h
	Mathematics/Figures.h

	Rendering/BatchQueue.h
	Rendering/RenderPasses/RenderingPass.h
	Rendering/RenderPasses/StandardRenderingPass.h

	Scenegraph/Node.h
	Scenegraph/Group.h
	Scenegraph/Light.h
	Scenegraph/Switch.h
	Scenegraph/Camera.h
	Scenegraph/Geometry.h
	Scenegraph/Scene.h

	Visitors/Visitor.h
	Visitors/FindNodes.h
	Visitors/FetchLights.h
	Visitors/DumpVisitor.h
	Visitors/LambdaVisitor.h
	Visitors/PrintNameVisitor.h
	Visitors/ChildrenCounterVisitor.h
	Visitors/FetchCameras.h
	Visitors/ComputeBatchQueue.h

	Utils/Easing.h
	Utils/Macros.h
	Utils/Layer.h

	Clock.h
)

set( LAVAENGINE_SOURCES

	#Mathematics/Color.cpp
	Mathematics/Frustum.cpp
	Mathematics/Mathf.cpp
	Mathematics/Ray.cpp
	Mathematics/Spherical.cpp

	Rendering/BatchQueue.cpp
	Rendering/RenderPasses/RenderingPass.cpp
	Rendering/RenderPasses/StandardRenderingPass.cpp
	
	Scenegraph/Node.cpp
	Scenegraph/Group.cpp
	Scenegraph/Light.cpp
	Scenegraph/Switch.cpp
	Scenegraph/Camera.cpp
	Scenegraph/Geometry.cpp
	Scenegraph/Scene.cpp

	Visitors/Visitor.cpp
	Visitors/FindNodes.cpp
	Visitors/FetchLights.cpp
	Visitors/DumpVisitor.cpp
	Visitors/LambdaVisitor.cpp
	Visitors/PrintNameVisitor.cpp
	Visitors/ChildrenCounterVisitor.cpp
	Visitors/FetchCameras.cpp
	Visitors/ComputeBatchQueue.cpp

	Utils/Easing.cpp
	Utils/Layer.cpp

	Clock.cpp
)

set( LAVAENGINE_HEADERS
)

set( LAVAENGINE_MOC_HEADERS
)

if( LAVAENGINE_WITH_COMPONENTS )
	list( APPEND LAVAENGINE_PUBLIC_HEADERS 
		Components/Component.h
		#Components/LambdaComponent.h
		Components/StateMachineComponent.h
		Components/RotateComponent.h

		Visitors/StartComponents.h
		Visitors/UpdateComponents.h
	)

	list( APPEND LAVAENGINE_SOURCES
		Components/Component.cpp
		#Components/LambdaComponent.cpp
		Components/StateMachineComponent.cpp
		Components/RotateComponent.cpp

		Visitors/StartComponents.cpp
		Visitors/UpdateComponents.cpp
	)
endif( )

set( LAVAENGINE_LINK_LIBRARIES
	lava
	lavaUtils
	lavaRenderer
	## ${GLM_LIBRARIES}		Required by lavaUtils
	## ${ASSIMP_LIBRARIES}	Required by lavaUtils
)
set( LAVAENGINE_INCLUDE_NAME lavaEngine )
set( LAVAENGINE_NAMESPACE lavaEngine )
common_library( lavaEngine )
