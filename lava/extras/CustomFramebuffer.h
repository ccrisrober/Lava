#pragma once

#include "../includes.hpp"

#include <lava/api.h>

#include "../VulkanResource.h"

namespace lava
{
  class Image;
  class ImageView;
  class Framebuffer;
  class RenderPass;
  class CommandBuffer;
  class Semaphore;
  namespace extras
  {
    class CustomFBO : public VulkanResource
    {
    public:
      // Framebuffer for offscreen rendering
      struct FramebufferAttachment
      {
        std::shared_ptr<lava::Image> image;
        std::shared_ptr<lava::ImageView> view;
        vk::ImageLayout finalLayout;
        vk::Format format;
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

        bool hasStencil( void )
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
      LAVA_API
      CustomFBO( DeviceRef dev, uint32_t width, uint32_t height );
      LAVA_API
      void addColorAttachmentt( vk::Format format );
      LAVA_API
      void addColorDepthAttachment( vk::Format format );  // TODO: Change method name or something :P
      LAVA_API
      void addDepthAttachment( vk::Format format );
      LAVA_API
      void build( void );
    private:
      void createAttachment( FramebufferAttachment& fatt, vk::Format format,
        vk::ImageUsageFlags usage );
    protected:
      uint32_t _width, _height;
      FramebufferAttachment _depthAttachment;
      bool hasDepth = false;
    public:
      LAVA_API
        const std::shared_ptr<ImageView> view( uint32_t i )
      {
        return _colorAttachments[ i ].view;
      }
      LAVA_API
      const vk::Sampler sampler( uint32_t )
      {
        return colorSampler;
      }
      std::shared_ptr< Framebuffer > _fbo;
      std::vector<FramebufferAttachment> _colorAttachments;
      std::shared_ptr<RenderPass> renderPass;
      vk::Sampler colorSampler;
      std::shared_ptr<CommandBuffer> commandBuffer;
      std::shared_ptr<Semaphore> semaphore;
    };
  }
}