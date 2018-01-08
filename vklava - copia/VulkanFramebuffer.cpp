#include "VulkanFramebuffer.h"
#include <assert.h>

namespace lava
{
  VulkanFramebuffer::VulkanFramebuffer( VulkanDevicePtr device, 
    const VULKAN_FRAMEBUFFER_DESC& desc )
    : VulkanResource( device )
  {
    _colorReferences.reserve( desc.numColorAttachments );
    uint32_t attachmentIdx = 0;
    for ( uint32_t i = 0; i < desc.numColorAttachments; ++i )
    {
		vk::AttachmentDescription colorAttachment;
      colorAttachment.format = desc.colorFormats[ i ];
      colorAttachment.samples = vk::SampleCountFlagBits::e1;
      colorAttachment.loadOp = vk::AttachmentLoadOp::eClear; // THIS -> DONT CLEAR vk::AttachmentLoadOp::eDontCare;
      colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
      colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
      colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
      colorAttachment.initialLayout = vk::ImageLayout::eUndefined;

      //if ( desc.offscreen )
      //  attachmentDesc.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
      //else
      colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
      _attachments.emplace_back( colorAttachment );

  vk::AttachmentReference ref;
      ref.attachment = attachmentIdx;
      ref.layout = vk::ImageLayout::eColorAttachmentOptimal;
      _colorReferences.emplace_back( ref );

      ++attachmentIdx;
    }

    if ( desc.hasDepth )
    {
	vk::AttachmentDescription depthAttachment;
      depthAttachment.format = desc.depthFormat;
      depthAttachment.samples = vk::SampleCountFlagBits::e1;
      depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
      depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
      depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
      depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
      depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
      depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

      _attachments.emplace_back( depthAttachment );

      _depthReferences.attachment = attachmentIdx;
      _depthReferences.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

      ++attachmentIdx;
    }

    _subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    _subpassDesc.colorAttachmentCount = _colorReferences.size( );
    _subpassDesc.inputAttachmentCount = 0;
    _subpassDesc.pInputAttachments = nullptr;
    _subpassDesc.preserveAttachmentCount = 0;
    _subpassDesc.pPreserveAttachments = nullptr;
    _subpassDesc.pResolveAttachments = nullptr;

    if ( desc.numColorAttachments > 0 )
      _subpassDesc.pColorAttachments = _colorReferences.data( );
    else
      _subpassDesc.pColorAttachments = nullptr;

    if ( desc.hasDepth )
      _subpassDesc.pDepthStencilAttachment = &_depthReferences;
    else
      _subpassDesc.pDepthStencilAttachment = nullptr;



    // Subpass dependencies for layout transitions
    _dependencies[ 0 ].srcSubpass = VK_SUBPASS_EXTERNAL;
    _dependencies[ 0 ].dstSubpass = 0;
    _dependencies[ 0 ].srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
    _dependencies[ 0 ].dstStageMask = vk::PipelineStageFlagBits::eAllCommands;
    _dependencies[ 0 ].srcAccessMask = vk::AccessFlagBits::eMemoryRead 
		| vk::AccessFlagBits::eShaderRead;
    _dependencies[ 0 ].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead 
		| vk::AccessFlagBits::eColorAttachmentWrite |
      vk::AccessFlagBits::eDepthStencilAttachmentRead 
		| vk::AccessFlagBits::eDepthStencilAttachmentWrite 
		| vk::AccessFlagBits::eShaderRead;
    //_dependencies[ 0 ].dependencyFlags = 0;

    _dependencies[ 1 ].srcSubpass = 0;
    _dependencies[ 1 ].dstSubpass = VK_SUBPASS_EXTERNAL;
    _dependencies[ 1 ].srcStageMask = vk::PipelineStageFlagBits::eAllCommands;
    _dependencies[ 1 ].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    _dependencies[ 1 ].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead 
		| vk::AccessFlagBits::eColorAttachmentWrite 
		| vk::AccessFlagBits::eDepthStencilAttachmentRead |
      vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eShaderRead;
    _dependencies[ 1 ].dstAccessMask = vk::AccessFlagBits::eMemoryRead 
		| vk::AccessFlagBits::eShaderRead;
    //_dependencies[ 1 ].dependencyFlags = 0;

    // Create render pass and frame buffer create infos
    _renderPassCI.attachmentCount = static_cast<uint32_t>( _attachments.size( ) );;
    _renderPassCI.pAttachments = _attachments.data( );
    _renderPassCI.subpassCount = 1;
    _renderPassCI.pSubpasses = &_subpassDesc;
    _renderPassCI.dependencyCount = 2;
    _renderPassCI.pDependencies = _dependencies;

    /*_framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    _framebufferCI.pNext = nullptr;
    _framebufferCI.flags = 0;
    _framebufferCI.renderPass = VK_NULL_HANDLE;
    _framebufferCI.attachmentCount = static_cast<uint32_t>( _attachmentViews.size( ) );
    _framebufferCI.pAttachments = _attachmentViews.data( );
    _framebufferCI.width = desc.width;
    _framebufferCI.height = desc.height;
    _framebufferCI.layers = 1; // desc.layers;*/


    vk::Device dev = _device->getLogical( );
	renderPass = dev.createRenderPass(&_renderPassCI);

    _framebufferCI.renderPass = renderPass;

    /*result = vkCreateFramebuffer( dev, &_framebufferCI, nullptr, &framebuffer );
    assert( result == VK_SUCCESS );*/
  }
  VulkanFramebuffer::~VulkanFramebuffer( void )
  {
    /*VkDevice device = _device->getLogical( );
    
    vkDestroyFramebuffer( device, framebuffer, nullptr );
    vkDestroyRenderPass( device, renderPass, nullptr );*/
  }
}
