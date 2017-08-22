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
      VkAttachmentDescription colorAttachment;
      colorAttachment.flags = 0;
      colorAttachment.format = desc.colorFormats[ i ];
      colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
      colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // THIS -> DONT CLEAR VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

      //if ( desc.offscreen )
      //  attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      //else
      colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      _attachments.emplace_back( colorAttachment );

      VkAttachmentReference ref;
      ref.attachment = attachmentIdx;
      ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      _colorReferences.emplace_back( ref );

      ++attachmentIdx;
    }

    if ( desc.hasDepth )
    {
      VkAttachmentDescription depthAttachment;
      depthAttachment.flags = 0;
      depthAttachment.format = desc.depthFormat;
      depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
      depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      _attachments.emplace_back( depthAttachment );

      _depthReferences.attachment = attachmentIdx;
      _depthReferences.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      ++attachmentIdx;
    }

    _subpassDesc.flags = 0;
    _subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
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
    _dependencies[ 0 ].srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    _dependencies[ 0 ].dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    _dependencies[ 0 ].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
    _dependencies[ 0 ].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
    _dependencies[ 0 ].dependencyFlags = 0;

    _dependencies[ 1 ].srcSubpass = 0;
    _dependencies[ 1 ].dstSubpass = VK_SUBPASS_EXTERNAL;
    _dependencies[ 1 ].srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    _dependencies[ 1 ].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    _dependencies[ 1 ].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
    _dependencies[ 1 ].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
    _dependencies[ 1 ].dependencyFlags = 0;

    // Create render pass and frame buffer create infos
    _renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    _renderPassCI.pNext = nullptr;
    _renderPassCI.flags = 0;
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


    VkDevice dev = _device->getLogical( );
    VkResult result = vkCreateRenderPass( dev, &_renderPassCI, nullptr, &renderPass );
    assert( result == VK_SUCCESS );

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
