#pragma once

#include "../includes.hpp"

#include "../VulkanResource.h"

namespace lava
{
  namespace extras
  {
    struct FramebufferAttachment
    {
      vk::Image image;
      vk::DeviceMemory memory;
      vk::ImageView view;
      vk::Format format;
      vk::ImageSubresourceRange subresourceRange;
      vk::AttachmentDescription description;

      bool hasDepth( void )
      {
        static std::vector<vk::Format> formats =
        {
          vk::Format::eD16Unorm,
          vk::Format::eX8D24UnormPack32,
          vk::Format::eD32Sfloat,
          vk::Format::eD16UnormS8Uint,
          vk::Format::eD24UnormS8Uint,
          vk::Format::eD32SfloatS8Uint
        };
        return std::find(
          formats.begin( ), formats.end( ), format ) != std::end( formats );
      }

      bool hasStencil( )
      {
        static std::vector<vk::Format> formats =
        {
          vk::Format::eS8Uint,
          vk::Format::eD16UnormS8Uint,
          vk::Format::eD24UnormS8Uint,
          vk::Format::eD32SfloatS8Uint
        };
        return std::find(
          formats.begin( ), formats.end( ), format ) != std::end( formats );
      }
      bool isDepthStencil( void )
      {
        return( hasDepth( ) || hasStencil( ) );
      }
    };
    struct AttachmentCreateInfo
    {
      uint32_t width, height;
      uint32_t layerCount;
      vk::Format format;
      vk::ImageUsageFlags usage;
    };

    class CustomFramebuffer: public VulkanResource
    {
    public:
      CustomFramebuffer( const DeviceRef& device );
      ~CustomFramebuffer( void );
      uint32_t addAttachment( const AttachmentCreateInfo ci );
      void createRenderPass( void );

      uint32_t width, height;
      vk::Framebuffer framebuffer;
      vk::RenderPass renderPass;
      vk::Sampler sampler;
      std::vector<FramebufferAttachment> attachments;
    };
  }
}