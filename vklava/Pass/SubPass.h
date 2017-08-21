#pragma once

class Subpass
{
public:
protected:
	void addColorAttachment( uint32_t index )
	{
		colorAttachments.emplace_back( index, vk::ImageLayout::eColorAttachmentOptimal );
	}
	void addDepthStencilAttachment( uint32_t index )
	{
		depthStencilAttachment = vk::AttachmentReference( index, vk::ImageLayout::eDepthStencilAttachmentOptimal );
		hasDepthStencil = true;
	}
private:
	bool hasDepthStencil = false;
	std::vector< vk::AttachmentReference > colorAttachments;
	vk::AttachmentReference depthStencilAttachment;
};