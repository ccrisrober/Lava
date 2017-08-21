#pragma once

class RenderPass: public vk::UniqueRenderPass
{
public:
	RenderPass( ) = default;
	RenderPass( vk::Device device );

	std::shared_ptr< Subpass > getFirstSubpass( void ) const;
	std::shared_ptr< Subpass > getLastSubpass( void ) const
	{

	}
};