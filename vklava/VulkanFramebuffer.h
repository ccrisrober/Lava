#ifndef __VKLAVA_VULKANFRAMEBUFFER__
#define __VKLAVA_VULKANFRAMEBUFFER__

#include "VulkanResource.h"
#include "VulkanTexture.h"

namespace lava
{
  struct VulkanFramebufferAttachment
  {
    VulkanImage* image = nullptr;
    uint32_t baseLayer = 0;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    uint32_t index = 0;
  };

  class VulkanFramebuffer :
    public VulkanResource
  {
  public:
    VulkanFramebuffer( VulkanDevicePtr device );
    ~VulkanFramebuffer( void );

    void addColorAttachment( VkFormat format, VkSampleCountFlagBits sampleFlags, 
      bool offscreen )
    {
      VkAttachmentDescription attachmentDesc = { };
      attachmentDesc.flags = 0;
      attachmentDesc.format = format;
      attachmentDesc.samples = sampleFlags;
      attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

      if ( offscreen )
      {
        attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      }
      else
      {
        attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      }
      colorAttachments.push_back( attachmentDesc );

      VkAttachmentReference ref = { };
      ref.attachment = _numAttachments;
      ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      colorAttachmentsRef.push_back( ref );
      _numAttachments++;
    }
    void addDepthAttachment( VkFormat format, VkSampleCountFlagBits sampleFlags )
    {
      if ( hasDepth )
      {
        return;
      }
      depthAttachment = { };
      depthAttachment.flags = 0;
      depthAttachment.format = format;
      depthAttachment.samples = sampleFlags;
      depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      VkAttachmentReference depthAttachmentRef = { };
      depthAttachmentRef.attachment = _numAttachments;
      depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      _numAttachments++;

      hasDepth = true;
    }

    /*void complete( void )
    {
      _framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      _framebufferCI.pNext = nullptr;
      _framebufferCI.flags = 0;
      _framebufferCI.renderPass = VK_NULL_HANDLE;
      _framebufferCI.attachmentCount = _numAttachments;
      _framebufferCI.pAttachments = _attachmentViews;
      _framebufferCI.width = _width;
      _framebufferCI.height = _height;
      _framebufferCI.layers = 1;
    }*/

  /*protected:
    uint32_t _numColorAttachments;
    uint32_t _numLayers;*/

    uint32_t _numAttachments = 0;
    uint32_t _width;
    uint32_t _height;
    uint32_t _depth;
    bool hasDepth = false;
    VkRenderPass _renderPass;
    VkFramebuffer _framebuffer;
    VkFramebufferCreateInfo _framebufferCI;

    std::vector< VkAttachmentDescription > colorAttachments;
    std::vector< VkAttachmentReference > colorAttachmentsRef;
    VkAttachmentDescription depthAttachment;
    VkAttachmentReference depthAttachmentRef;
  };
}

#endif /* __VKLAVA_VULKANFRAMEBUFFER__ */